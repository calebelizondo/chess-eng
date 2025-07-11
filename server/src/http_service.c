#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "http_service.h"
#include "ws_service.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found"
#define RESPONSE_200 "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n"


const char* get_mime_type(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".wasm")) return "application/wasm"; 
    return "application/octet-stream";
}

void serve_file(int client_fd, const char* path) {
    char full_path[256] = "../public"; 
    strcat(full_path, path); 

    FILE* file = fopen(full_path, "rb");

    if (!file) {
        const char* not_found = RESPONSE_404;
        write(client_fd, not_found, strlen(not_found)); 
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char* content = malloc(fsize);
    fread(content, 1, fsize, file);
    fclose(file);

    const char* mime = get_mime_type(full_path);
    char header[512];

    snprintf(header, sizeof(header), RESPONSE_200, mime, fsize);

    write(client_fd, header, strlen(header));
    write(client_fd, content, fsize);

    free(content);
}


void* http_service(void* args) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET, 
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    listen(server_fd, 10);

    printf("server running on port: %d", PORT);


    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;

        char buffer[BUFFER_SIZE] = {0};
        read(client_fd, buffer, sizeof(buffer) - 1);

        char method[8], path[256];
        sscanf(buffer, "%s %s", method, path);
        if (strcmp(method, "GET") == 0) {

            if (strcmp(path, WS_PATH) == 0) {
                //todo, dispatch to ws_service
            }else {
                serve_file(client_fd, path); 
            }
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}