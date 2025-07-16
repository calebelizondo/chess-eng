#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include "http_service.h"
#include "matching_service.h"
#include <libwebsockets.h>
#include <map>
#include "matching_service.h"
#include <json.hpp>
using json = nlohmann::json;

#define PORT 8080
#define BUFFER_SIZE 4096
#define READ_CHUNK_SIZE 4096
#define STATIC_DIR "../public"


std::map<std::string, struct lws*>client_map; 

const char* get_mime_type(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".wasm")) return "application/wasm";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

void error_respond(struct lws *wsi) {
    char msg[128] = "{\"type\":\"error\",\"id\":\"Peer ID does not exist\"}";
    unsigned char buf[LWS_PRE + 128];
    size_t n = snprintf((char*)&buf[LWS_PRE], 128, "%s", msg);
    lws_write(wsi, &buf[LWS_PRE], n, LWS_WRITE_TEXT);
}


int forward(std::string to, std::string message, size_t len) {
    if (client_map[to]) {
        struct lws* peer = client_map[to];
        unsigned char buf[LWS_PRE + BUFFER_SIZE];
        std::memcpy(&buf[LWS_PRE], message.c_str(), len);

        lws_write(peer, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
        return 0;
    }

    return -1;
}

int send_404(struct lws *wsi) {
    const char *not_found_body = "404 Not Found";
    unsigned char buffer[LWS_PRE + 256], *p = &buffer[LWS_PRE], *end = p + 256;

    // Add status line and Content-Type
    if (lws_add_http_common_headers(wsi, HTTP_STATUS_NOT_FOUND, "text/plain", strlen(not_found_body), &p, end)) {
        return -1;
    }

    if (lws_finalize_http_header(wsi, &p, end)) {
        return -1;
    }

    // Send headers
    if (lws_write(wsi, &buffer[LWS_PRE], p - &buffer[LWS_PRE], LWS_WRITE_HTTP_HEADERS) < 0) {
        return -1;
    }

    // Send body
    if (lws_write(wsi, (unsigned char*)not_found_body, strlen(not_found_body), LWS_WRITE_HTTP_FINAL) < 0) {
        return -1;
    }

    return 0;
}



static int callback_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {

            /* on initial connection, send client id back*/
            std::string client_id = to_base36(hash_lws_ptr_id(wsi), 5);
            client_map[client_id] = wsi;

            char msg[128];
            snprintf(msg, sizeof(msg), "{\"type\":\"init\",\"id\":\"%s\"}", client_id.c_str());
            unsigned char buf[LWS_PRE + 128];
            size_t n = snprintf((char*)&buf[LWS_PRE], 128, "%s", msg);
            lws_write(wsi, &buf[LWS_PRE], n, LWS_WRITE_TEXT);

            break;
        }
        case LWS_CALLBACK_RECEIVE: {

            std::string type;
            std::string to;
            std::string message((char*) in, len);

            try {
                json payload = json::parse(message);
                type = payload["type"];
                to = payload["to"];

            } catch (const std::exception& e) {
                printf("JSON parse error");
            }

            if (type.length() == 0 || to.length() == 0) break;

            if (type == "offer" || type == "answer" || type == "ice") {
                if (forward(to, message, len) != 0) error_respond(wsi);
            }

            break;
        }
        case LWS_CALLBACK_CLOSED: {
            //printf("WS connection closed");
            break;
        } 
        default: {
            break;
        }
    }

    return 0;
}
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            printf("file requested: %s\n", (const char*) in);
            fflush(stdout);

            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s%s", STATIC_DIR, (const char*) in);

            printf("filepath: %s\n", (const char*) filepath);
            fflush(stdout);

            int fd = open(filepath, O_RDONLY);
            if (fd < 0) {
                printf("File not found: %s\n", filepath);
                fflush(stdout);
                return send_404(wsi);
            }

            struct stat file_stat;
            if (fstat(fd, &file_stat) < 0) {
                printf("File stat noit found: %s\n", filepath);
                fflush(stdout);
                close(fd);
                return send_404(wsi);
            }

            const char* mime = get_mime_type(filepath);
            unsigned char buffer[LWS_PRE + 2048], *p = &buffer[LWS_PRE], *end = p + 2048;

            if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, mime, file_stat.st_size, &p, end)) {
                printf("adding headers failed %s\n", filepath);
                fflush(stdout);
                close(fd);
                return -1;
            }

            if (lws_add_http_header_by_name(
                    wsi,
                    (const unsigned char*) "cross-origin-opener-policy",
                    (const unsigned char*) "same-origin",
                    strlen("same-origin"), &p, end) ||

                lws_add_http_header_by_name(
                    wsi,
                    (const unsigned char*) "cross-origin-embedder-policy",
                    (const unsigned char*) "require-corp",
                    strlen("require-corp"), &p, end)) {
                printf("adding custom headers failed: %s\n", filepath);
                fflush(stdout);
                close(fd);
                return -1;
            }


            /* TEMPORARY */
            if (lws_add_http_header_by_name(
                    wsi,
                    (const unsigned char*) "content-security-policy",
                    (const unsigned char*) "default-src 'self'; script-src 'self' 'unsafe-eval'; style-src 'self'; connect-src 'self' ws:;",
                    strlen("default-src 'self'; script-src 'self' 'unsafe-eval'; style-src 'self'; connect-src 'self' ws:;"),
                    &p, end)) {
                close(fd);
                return -1;
            }

            if (lws_finalize_http_header(wsi, &p, end)) {
                printf("finalizing headers failed: %s\n", filepath);
                fflush(stdout);
                close(fd);
                return -1;
            }

            if (lws_write(wsi, &buffer[LWS_PRE], p - &buffer[LWS_PRE], LWS_WRITE_HTTP_HEADERS) < 0) {
                printf("write failed: %s\n", filepath);
                fflush(stdout);
                close(fd);
                return -1;
            }

            char file_buf[READ_CHUNK_SIZE];
            ssize_t bytes_read;

            while ((bytes_read = read(fd, file_buf, READ_CHUNK_SIZE)) > 0) {
                lws_write_protocol flags = (bytes_read < READ_CHUNK_SIZE) ? LWS_WRITE_HTTP_FINAL : LWS_WRITE_HTTP;

                if (lws_write(wsi, (unsigned char*)file_buf, bytes_read, flags) < 0) {
                    close(fd);
                    return -1;
                }

                if (flags == LWS_WRITE_HTTP_FINAL) break;
            }
            printf("stream done: %s\n", filepath);
            fflush(stdout);
            close(fd);
            return 0;
        }

        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
            // Close the connection after file has been sent
            return -1;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "http-only",
        callback_http,
        0,
        0,
    },
    {
        "ws-protocol",
        callback_ws,
        0,
        4096,
    },
    { NULL, NULL, 0, 0 } 
};


void* http_service(void* args) {
    struct lws_context_creation_info info; 
    memset(&info, 0, sizeof(info)); 

    info.port = PORT; 
    info.protocols = protocols; 
    info.gid = -1; 
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    struct lws_context *context = lws_create_context(&info); 

    if (!context) {
        fprintf(stderr, "failed to create context"); 
        return NULL;
    }

    printf("server running!");
    while (1) {
        lws_service(context, 0);
    }

    lws_context_destroy(context);
    return NULL;
}
