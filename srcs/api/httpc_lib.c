/**
 * @file httpc_lib.c
 * @brief Micro Framework HTTP
 * @version 0.0.1
 * @author Aldantas
 * 
 */

#include "../../includes/httpc.h"
#include "../../includes/http.h"
#include "../../includes/server/server.h"
#include "../../includes/constants/constants.h"
#include "../../includes/handlers/methods.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 10
#define DEFAULT_MAX_CLIENTS 100

static httpc_config_t g_config = {0};
static int g_server_socket = -1;
static int g_running = 0;
static pthread_t g_main_thread = {0};

static char* string_dup(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char* dup = malloc(len + 1);
    if (!dup) return NULL;
    
    strcpy(dup, str);
    return dup;
}

static int validate_config(const httpc_config_t* config) {
    if (!config) return -1;
    
    if (config->port <= 0 || config->port > 65535) return -1;
    if (config->backlog <= 0) return -1;
    if (config->max_clients <= 0) return -1;
    if (!config->host) return -1;
    
    return 0;
}

static void apply_default_config(httpc_config_t* config) {
    if (config->port <= 0 || config->port > 65535) {
        config->port = PORT;
    }
    
    if (config->backlog <= 0) {
        config->backlog = BACKLOG;
    }
    
    if (config->max_clients <= 0) {
        config->max_clients = MAX_CLIENTS;
    }
    
    if (!config->host) {
        config->host = DEFAULT_HOST;
    }
}

static const char* get_status_text(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}


static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        g_running = 0;
        
        if (g_server_socket != -1) {
            close(g_server_socket);
            g_server_socket = -1;
        }
    }
}


static void* server_main_loop(void* arg) {
    (void)arg;
    
    if (g_config.on_request) {
        g_config.on_request("SERVER", "STARTED", "Server is running");
    }
    
    main_handler(g_server_socket);
    return (NULL);
}


int httpc_init(void) {
    g_config.port = PORT;
    g_config.backlog = BACKLOG;
    g_config.max_clients = MAX_CLIENTS;
    g_config.host = DEFAULT_HOST;
    g_config.on_request = NULL;
    g_config.on_error = NULL;
    
    router_init();
    
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        return -1;
    }
    
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        return -1;
    }
    
    return 0;
}

int httpc_configure(const httpc_config_t* config) {
    if (!config) {
        errno = EINVAL;
        return -1;
    }
    
    if (validate_config(config) != 0) {
        errno = EINVAL;
        return -1;
    }
    
    g_config = *config;
    apply_default_config(&g_config);
    
    return 0;
}

int httpc_add_route(const char* method, const char* path, route_handler handler) {
    if (!method || !path || !handler) {
        errno = EINVAL;
        return -1;
    }
    
    router_add(method, path, handler);
    
    return 0;
}

int httpc_start(void) {
    struct sockaddr_in serverAddress;
    
    if (g_running) {
        errno = EALREADY;
        return -1;
    }
    
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
        g_server_socket = -1;
        return -1;
    }
    
    if (start_listening(g_server_socket, g_config.backlog) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to start listening");
        }
        close(g_server_socket);
        g_server_socket = -1;
        return -1;
    }
    
    g_running = 1;
    
    if (pthread_create(&g_main_thread, NULL, server_main_loop, NULL) != 0) {
        if (g_config.on_error) {
            g_config.on_error("Failed to create server thread");
        }
        close(g_server_socket);
        g_server_socket = -1;
        g_running = 0;
        return -1;
    }
    
    printf("HTTP.c server started on %s:%d\n", g_config.host, g_config.port);
    return 0;
}

int httpc_stop(void) {
    if (!g_running) {
        return 0;
    }
    
    g_running = 0;
    
    if (g_server_socket != -1) {
        close(g_server_socket);
        g_server_socket = -1;
    }
    
    if (g_main_thread) {
        pthread_join(g_main_thread, NULL);
        memset(&g_main_thread, 0, sizeof(pthread_t));
    }
    
    printf("HTTP.c server stopped\n");
    return 0;
}

void httpc_cleanup(void) {
    httpc_stop();
}

httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body) {
    httpc_response_t* response = malloc(sizeof(httpc_response_t));
    if (!response) {
        return NULL;
    }
    
    response->status_code = status_code;
    response->content_type = content_type ? string_dup(content_type) : string_dup("text/plain");
    response->body = body ? string_dup(body) : string_dup("");
    response->headers = NULL;
    
    if (!response->content_type || !response->body) {
        httpc_free_response(response);
        return NULL;
    }
    
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
    if (!response) {
        return NULL;
    }
    
    const char* status_text = get_status_text(response->status_code);
    
    size_t total_size = 0;
    
    total_size += snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", response->status_code, status_text);
    
    total_size += snprintf(NULL, 0, "Content-Type: %s\r\n", response->content_type);
    total_size += snprintf(NULL, 0, "Content-Length: %zu\r\n", strlen(response->body));
    
    if (response->headers) {
        total_size += strlen(response->headers);
        if (response->headers[strlen(response->headers) - 1] != '\n') {
            total_size += 2;
        }
    }
    
    total_size += 4;
    total_size += strlen(response->body);
    
    char* result = malloc(total_size + 1);
    if (!result) {
        return NULL;
    }
    
    char* ptr = result;
    
    ptr += sprintf(ptr, "HTTP/1.1 %d %s\r\n", response->status_code, status_text);
    ptr += sprintf(ptr, "Content-Type: %s\r\n", response->content_type);
    ptr += sprintf(ptr, "Content-Length: %zu\r\n", strlen(response->body));
    
    if (response->headers) {
        ptr += sprintf(ptr, "%s", response->headers);
        if (response->headers[strlen(response->headers) - 1] != '\n') {
            ptr += sprintf(ptr, "\r\n");
        }
    }
    
    ptr += sprintf(ptr, "\r\n");
    ptr += sprintf(ptr, "%s", response->body);
    
    return result;
}

void httpc_set_header(httpc_response_t* response, const char* key, const char* value) {
    if (!response || !key || !value) {
        return;
    }
    
    size_t header_size = strlen(key) + strlen(value) + 4;
    
    if (response->headers) {
        size_t current_size = strlen(response->headers);
        size_t new_size = current_size + header_size;
        
        char* new_headers = realloc(response->headers, new_size + 1);
        if (!new_headers) {
            return;
        }
        
        response->headers = new_headers;
        sprintf(response->headers + current_size, "%s: %s\r\n", key, value);
    } else {
        response->headers = malloc(header_size + 1);
        if (!response->headers) {
            return;
        }
        
        sprintf(response->headers, "%s: %s\r\n", key, value);
    }
}

int httpc_is_running(void) {
    return g_running;
}

const httpc_config_t* httpc_get_config(void) {
    return &g_config;
}

int httpc_get_server_socket(void) {
    return g_server_socket;
}
