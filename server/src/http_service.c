#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "http_service.h"
#include "ws_connect.h"
#include <libwebsockets.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define STATIC_DIR "../public"


const char* get_mime_type(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".wasm")) return "application/wasm"; 
    return "application/octet-stream";
}


static int callback_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("WS connection established");
            /*
                Assign a code
                Send a code
            */
            break;
        case LWS_CALLBACK_RECEIVE:
            printf("WS received %.*s\n", (int) len, (char*) in); 
            lws_write(wsi, (unsigned char*) in, len, LWS_WRITE_TEXT); 
            break;
        case LWS_CALLBACK_CLOSED:
            printf("WS connection closed");
            break; 
        default: 
            break;
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
