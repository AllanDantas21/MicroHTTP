#include "../../includes/api/response_helpers.h"
#include "../../includes/api/response.h"
#include "../../includes/core/error_handling.h"
#include "../../includes/json/json_utils.h"
#include "../../srcs/json/cJSON.h"
#include <string.h>
#include <stdlib.h>

httpc_response_t* httpc_json(int status, cJSON* json) {
    if (!json) {
        return httpc_create_response(status, "application/json", "{}");
    }
    
    char* json_string = cJSON_Print(json);
    if (!json_string) {
        cJSON_Delete(json);
        return httpc_create_response(status, "application/json", "{}");
    }
    
    httpc_response_t* response = httpc_create_response(status, "application/json", json_string);
    free(json_string);
    cJSON_Delete(json);
    
    return response;
}

httpc_response_t* httpc_json_ok(cJSON* json) {
    if (!json) {
        return httpc_json_error(500, "JSON object is null");
    }
    return httpc_json(200, json);
}

httpc_response_t* httpc_json_error(int status, const char* message) {
    cJSON* json = httpc_json_create_object();
    if (!json) {
        return httpc_create_response(status, "application/json", "{\"error\":\"Internal error\"}");
    }
    
    httpc_json_add_string(json, "error", message ? message : "Unknown error");
    httpc_json_add_number(json, "status", status);
    
    return httpc_json(status, json);
}

httpc_response_t* httpc_html(int status, const char* html) {
    return httpc_create_response(status, "text/html", html ? html : "");
}

httpc_response_t* httpc_text(int status, const char* text) {
    return httpc_create_response(status, "text/plain", text ? text : "");
}

httpc_response_t* httpc_ok(const char* text) {
    return httpc_text(200, text);
}

httpc_response_t* httpc_not_found(const char* message) {
    return httpc_text(404, message ? message : "Not Found");
}

httpc_response_t* httpc_bad_request(const char* message) {
    return httpc_text(400, message ? message : "Bad Request");
}

httpc_response_t* httpc_internal_error(const char* message) {
    return httpc_text(500, message ? message : "Internal Server Error");
}

