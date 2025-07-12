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
#define STATIC_DIR "../public"


std::map<std::string, struct lws*>client_map; 

const char* get_mime_type(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".wasm")) return "application/wasm"; 
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

static int callback_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    printf("callback_ws: reason=%d\n", reason);
    printf("callback_established: %d\n", LWS_CALLBACK_ESTABLISHED);
    fflush(stdout);
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
            char filepath[512];
            const char *requested = (const char*) in;

            snprintf(filepath, sizeof(filepath), "%s%s", STATIC_DIR, requested); 
            const char* mime = get_mime_type(filepath);
            if (lws_serve_http_file(wsi, filepath, mime, NULL, 0)) {
                return -1;
            }

            return 0;
        }
        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
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
