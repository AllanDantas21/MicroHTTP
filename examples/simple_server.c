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

void on_request(const char* method, const char* path, const char* body) {
    printf("[INFO] %s %s\n", method, path);
    if (body && strlen(body) > 0) {
        printf("[BODY] %s\n", body);
    }
}

void on_error(const char* error) {
    fprintf(stderr, "[ERROR] %s\n", error);
}

int main(void) {
    if (init_logger_system() < 0) {
        return 1;
    }
    
    if (httpc_init() != 0) {
        cleanup_logger_system();
        return 1;
    }
    
    httpc_config_t config = {
        .port = 8080,
        .backlog = 10,
        .max_clients = 10,
        .host = "127.0.0.1",
        .on_request = on_request,
        .on_error = on_error
    };
    
    if (httpc_configure(&config) != 0) {
        cleanup_logger_system();
        httpc_cleanup();
        return 1;
    }
    
    httpc_add_route(&g_router, "GET", "", handle_home);
    httpc_add_route(&g_router, "GET", "status", handle_api_status);
    httpc_add_route(&g_router, "POST", "echo", handle_echo);
    
    if (httpc_start() != 0) {
        cleanup_logger_system();
        httpc_cleanup();
        return 1;
    }
    
    int result = httpc_run();
    cleanup_logger_system();
    return result;
}
