#include "../../includes/json/json_utils.h"
#include "../../srcs/json/cJSON.h"
#include <string.h>
#include <stdlib.h>

char* httpc_extract_json_body(const char* request_buffer) {
    if (!request_buffer) {
        return NULL;
    }
    
    char *body = strstr(request_buffer, "\r\n\r\n");
    if (!body) {
        return NULL;
    }
    
    body += 4;
    
    if (strlen(body) == 0) {
        return NULL;
    }
    
    char *json_body = malloc(strlen(body) + 1);
    if (!json_body) {
        return NULL;
    }
    
    strcpy(json_body, body);
    return json_body;
}

cJSON* httpc_parse_json(const char* json_string) {
    if (!json_string) {
        return NULL;
    }
    return cJSON_Parse(json_string);
}

httpc_response_t* httpc_create_json_response(int status_code, cJSON* json_object) {
    if (!json_object) {
        return NULL;
    }
    
    char *json_string = cJSON_Print(json_object);
    if (!json_string) {
        return NULL;
    }
    
    httpc_response_t* response = httpc_create_response(status_code, "application/json", json_string);
    free(json_string);
    
    return response;
}

httpc_response_t* httpc_create_json_error_response(int status_code, const char* error_message) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "error", error_message ? error_message : "Erro desconhecido");
    cJSON_AddNumberToObject(json, "status", status_code);
    
    httpc_response_t* response = httpc_create_json_response(status_code, json);
    
    cJSON_Delete(json);
    return response;
}

httpc_response_t* httpc_create_json_success_response(const char* message, cJSON* data) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "message", message ? message : "Sucesso");
    cJSON_AddBoolToObject(json, "success", 1);
    
    if (data) {
        cJSON_AddItemToObject(json, "data", data);
    }
    
    httpc_response_t* response = httpc_create_json_response(200, json);
    
    cJSON_Delete(json);
    return response;
}

const char* httpc_json_get_string(cJSON* json, const char* key) {
    if (!json || !key) {
        return NULL;
    }
    
    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!item || !cJSON_IsString(item)) {
        return NULL;
    }
    
    return item->valuestring;
}

double httpc_json_get_number(cJSON* json, const char* key, double default_value) {
    if (!json || !key) {
        return default_value;
    }
    
    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!item || !cJSON_IsNumber(item)) {
        return default_value;
    }
    
    return item->valuedouble;
}

int httpc_json_get_bool(cJSON* json, const char* key, int default_value) {
    if (!json || !key) {
        return default_value;
    }
    
    cJSON *item = cJSON_GetObjectItem(json, key);
    if (!item || !cJSON_IsBool(item)) {
        return default_value;
    }
    return cJSON_IsTrue(item);
}

int httpc_json_has_key(cJSON* json, const char* key) {
    if (!json || !key) {
        return 0;
    }
    
    return cJSON_HasObjectItem(json, key);
}

cJSON* httpc_json_create_object(void) {
    return cJSON_CreateObject();
}

cJSON* httpc_json_create_array(void) {
    return cJSON_CreateArray();
}

int httpc_json_add_string(cJSON* json, const char* key, const char* value) {
    if (!json || !key) {
        return 0;
    }
    return cJSON_AddStringToObject(json, key, value ? value : "") != NULL;
}

int httpc_json_add_number(cJSON* json, const char* key, double value) {
    if (!json || !key) {
        return 0;
    }
    return cJSON_AddNumberToObject(json, key, value) != NULL;
}

int httpc_json_add_bool(cJSON* json, const char* key, int value) {
    if (!json || !key) {
        return 0;
    }
    return cJSON_AddBoolToObject(json, key, value) != NULL;
}

int httpc_json_add_to_array(cJSON* array, cJSON* item) {
    if (!array || !item) {
        return 0;
    }
    return cJSON_AddItemToArray(array, item);
}

void httpc_json_free(cJSON* json) {
    if (json) {
        cJSON_Delete(json);
    }
}
