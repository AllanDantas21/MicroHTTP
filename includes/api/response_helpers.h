#ifndef RESPONSE_HELPERS_H
#define RESPONSE_HELPERS_H

#include "../structs/httpc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cJSON;
typedef struct cJSON cJSON;

httpc_response_t* httpc_json(int status, cJSON* json);
httpc_response_t* httpc_json_ok(cJSON* json);
httpc_response_t* httpc_json_error(int status, const char* message);
httpc_response_t* httpc_html(int status, const char* html);
httpc_response_t* httpc_text(int status, const char* text);
httpc_response_t* httpc_ok(const char* text);
httpc_response_t* httpc_not_found(const char* message);
httpc_response_t* httpc_bad_request(const char* message);
httpc_response_t* httpc_internal_error(const char* message);

#ifdef __cplusplus
}
#endif

#endif

