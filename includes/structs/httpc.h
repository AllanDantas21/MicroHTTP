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

typedef struct HttpHeader HttpHeader;

typedef struct QueryParam {
    char* name;
    char* value;
    struct QueryParam* next;
} QueryParam;

typedef struct PathParam {
    char* name;
    char* value;
    struct PathParam* next;
} PathParam;

typedef struct {
    char* method;
    char* path;
    char* body;
    char* headers;
    char* query_string;
    HttpHeader* parsed_headers;
    QueryParam* parsed_query_params;
    PathParam* path_params;
} httpc_request_t;

#endif
