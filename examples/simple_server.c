#include "../includes/httpc.h"
#include "../includes/http.h"
#include <stdio.h>

char* handle_home(const char* buffer) {
    (void)buffer;
    return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 89\r\n\r\n<html><body><h1>Bem-vindo ao HTTP.c!</h1><p>Um micro framework HTTP em C</p></body></html>";
}

char* handle_api_status(const char* buffer) {
    (void)buffer;
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 25\r\n\r\n{\"status\": \"running\"}";
}

char* handle_echo(const char* buffer) {
    static char response[1024];
    
    char *body = strstr(buffer, "\r\n\r\n");
    if (body) body += 4;
    
    if (body && strlen(body) > 0) {
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "Echo: %s", strlen(body), body);
    } else {
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "Echo: empty");
    }
    
    return response;
}

int main(void) {
    if (httpc_init() != 0) {
        return 1;
    }
    
    httpc_config_t config = {
        .port = 8080,
        .backlog = 10,
        .max_clients = 10,
        .host = "127.0.0.1"
    };
    
    if (httpc_configure(&config) != 0) {
        httpc_cleanup();
        return 1;
    }
    
    httpc_add_route(&g_router, "GET", "", handle_home);
    httpc_add_route(&g_router, "GET", "status", handle_api_status);
    httpc_add_route(&g_router, "POST", "echo", handle_echo);
    
    return httpc_start();
}
