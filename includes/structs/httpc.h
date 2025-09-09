#ifndef HTTPC_STRUCTS_H
#define HTTPC_STRUCTS_H

typedef struct {
    int port;
    int backlog;
    int max_clients;
    void (*on_request)(const char* method, const char* path, const char* body);
    void (*on_error)(const char* error);
} httpc_config_t;

typedef struct {
    int status_code;
    char* content_type;
    char* body;
    char* headers;
} httpc_response_t;

typedef struct {
    char* method;
    char* path;
    char* body;
    char* headers;
    char* query_string;
} httpc_request_t;

#endif
