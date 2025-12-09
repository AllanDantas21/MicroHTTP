#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include "../structs/httpc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HttpHeader {
    char* name;
    char* value;
    struct HttpHeader* next;
};

typedef struct HttpHeader HttpHeader;

int httpc_parse_request(const char* buffer, httpc_request_t* req);
void httpc_free_request(httpc_request_t* req);
const char* httpc_get_header_value(httpc_request_t* req, const char* name);
const char* httpc_get_header(httpc_request_t* req, const char* name);
const char* httpc_get_query_param(httpc_request_t* req, const char* name);
const char* httpc_get_body(httpc_request_t* req);
const char* httpc_get_method(httpc_request_t* req);
const char* httpc_get_path(httpc_request_t* req);

#ifdef __cplusplus
}
#endif

#endif

