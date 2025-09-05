#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "api/httpc.h"
#include "../structs/httpc.h"
#include "../../srcs/json/cJSON.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* httpc_extract_json_body(const char* request_buffer);
cJSON* httpc_parse_json(const char* json_string);
httpc_response_t* httpc_create_json_response(int status_code, cJSON* json_object);
httpc_response_t* httpc_create_json_error_response(int status_code, const char* error_message);
httpc_response_t* httpc_create_json_success_response(const char* message, cJSON* data);
const char* httpc_json_get_string(cJSON* json, const char* key);
double httpc_json_get_number(cJSON* json, const char* key, double default_value);
int httpc_json_get_bool(cJSON* json, const char* key, int default_value);
int httpc_json_has_key(cJSON* json, const char* key);
cJSON* httpc_json_create_object(void);
cJSON* httpc_json_create_array(void);
int httpc_json_add_string(cJSON* json, const char* key, const char* value);
int httpc_json_add_number(cJSON* json, const char* key, double value);
int httpc_json_add_bool(cJSON* json, const char* key, int value);
int httpc_json_add_to_array(cJSON* array, cJSON* item);
void httpc_json_free(cJSON* json);

#ifdef __cplusplus
}
#endif

#endif
