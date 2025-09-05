#ifndef RESPONSE_H
#define RESPONSE_H

#include "api/httpc.h"

httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body);
void httpc_free_response(httpc_response_t* response);
char* httpc_response_to_string(const httpc_response_t* response);
void httpc_set_header(httpc_response_t* response, const char* key, const char* value);
char* httpc_build_headers(int status_code, const char* content_type, size_t content_length, const char* extra_headers);
char* build_response(int status_code, const char* content_type, const char* body);

#endif
