#include "../includes/httpc.h"
#include "../includes/http.h"
#include "../includes/server/server.h"
#include "../includes/constants/constants.h"
#include "../includes/handlers/methods.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>

static char* string_dup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* dup = malloc(len + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

static httpc_config_t g_config = {0};
static int g_server_socket = -1;
static int g_running = 0;
static pthread_t g_main_thread;

static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        g_running = 0;
        if (g_server_socket != -1) {
            close(g_server_socket);
        }
    }
}

static void* server_main_loop(void* arg) {
    (void)arg;
    
    main_handler(g_server_socket);
    
    return NULL;
}

int httpc_init(void) {
    g_config.port = PORT;
    g_config.backlog = BACKLOG;
    g_config.max_clients = MAX_CLIENTS;
    g_config.host = "127.0.0.1";
    g_config.on_request = NULL;
    g_config.on_error = NULL;
    
    router_init();
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    return 0;
}

int httpc_configure(const httpc_config_t* config) {
    if (!config) return -1;
    
    g_config = *config;
    
    if (g_config.port <= 0 || g_config.port > 65535) {
        g_config.port = PORT;
    }
    
    if (g_config.backlog <= 0) {
        g_config.backlog = BACKLOG;
    }
    
    if (g_config.max_clients <= 0) {
        g_config.max_clients = MAX_CLIENTS;
    }
    
    return 0;
}

int httpc_add_route(const char* method, const char* path, route_handler handler) {
    if (!method || !path || !handler) return -1;
    
    router_add(method, path, handler);
    return 0;
}

int httpc_start(void) {
    struct sockaddr_in serverAddress;
    
    if (setup_server_address(&serverAddress, g_config.port) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to setup server address");
        }
        return -1;
    }
    
    g_server_socket = create_server_socket();
    if (g_server_socket < 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to create server socket");
        }
        return -1;
    }
    
    if (bind_server_socket(g_server_socket, &serverAddress) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to bind server socket");
        }
        close(g_server_socket);
        return -1;
    }
    
    if (start_listening(g_server_socket, g_config.backlog) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to start listening");
        }
        close(g_server_socket);
        return -1;
    }
    
    g_running = 1;
    
    if (pthread_create(&g_main_thread, NULL, server_main_loop, NULL) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to create server thread");
        }
        close(g_server_socket);
        return -1;
    }
    
    printf("HTTP.c server started on %s:%d\n", g_config.host, g_config.port);
    return 0;
}

int httpc_stop(void) {
    if (!g_running) return 0;
    
    g_running = 0;
    
    if (g_server_socket != -1) {
        close(g_server_socket);
        g_server_socket = -1;
    }
    
    if (g_main_thread) {
        pthread_join(g_main_thread, NULL);
    }
    
    printf("HTTP.c server stopped\n");
    return 0;
}

void httpc_cleanup(void) {
    httpc_stop();
}

httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body) {
    httpc_response_t* response = malloc(sizeof(httpc_response_t));
    if (!response) return NULL;
    
    response->status_code = status_code;
    response->content_type = content_type ? string_dup(content_type) : string_dup("text/plain");
    response->body = body ? string_dup(body) : string_dup("");
    response->headers = NULL;
    
    return response;
}

void httpc_free_response(httpc_response_t* response) {
    if (!response) return;
    
    free(response->content_type);
    free(response->body);
    free(response->headers);
    free(response);
}

char* httpc_response_to_string(const httpc_response_t* response) {
    if (!response) return NULL;
    
    char status_line[256];
    const char* status_text = "OK";
    
    switch (response->status_code) {
        case 200: status_text = "OK"; break;
        case 201: status_text = "Created"; break;
        case 400: status_text = "Bad Request"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s", 
             response->status_code, status_text);
    
    size_t total_size = strlen(status_line) + 4;
    total_size += strlen("Content-Type: ") + strlen(response->content_type) + 2;
    total_size += strlen("Content-Length: ") + 20 + 2;
    if (response->headers) {
        total_size += strlen(response->headers) + 2;
    }
    total_size += strlen(response->body);
    
    char* result = malloc(total_size + 1);
    if (!result) return NULL;
    
    char* ptr = result;
    
    ptr += sprintf(ptr, "%s\r\n", status_line);
    
    ptr += sprintf(ptr, "Content-Type: %s\r\n", response->content_type);
    ptr += sprintf(ptr, "Content-Length: %zu\r\n", strlen(response->body));
    
    if (response->headers) {
        ptr += sprintf(ptr, "%s\r\n", response->headers);
    }
    
    ptr += sprintf(ptr, "\r\n");
    ptr += sprintf(ptr, "%s", response->body);
    
    return result;
}

void httpc_set_header(httpc_response_t* response, const char* key, const char* value) {
    if (!response || !key || !value) return;
    
    if (response->headers) {
        size_t new_size = strlen(response->headers) + strlen(key) + strlen(value) + 4;
        char* new_headers = realloc(response->headers, new_size);
        if (new_headers) {
            response->headers = new_headers;
            sprintf(response->headers + strlen(response->headers), "%s: %s\r\n", key, value);
        }
    } else {
        size_t size = strlen(key) + strlen(value) + 4;
        response->headers = malloc(size);
        if (response->headers) {
            sprintf(response->headers, "%s: %s\r\n", key, value);
        }
    }
}
