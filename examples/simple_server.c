#include "../includes/httpc.h"
#include "../includes/http.h"
#include "../includes/json/json_utils.h"
#include <stdio.h>

char* handle_home(const char* buffer) {
    (void)buffer;
    return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 89\r\n\r\n<html><body><h1>Bem-vindo ao HTTP.c!</h1><p>Um micro framework HTTP em C</p></body></html>";
}

char* handle_api_status(const char* buffer) {
    (void)buffer;
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 25\r\n\r\n{\"status\": \"running\"}";
}

char* handle_json_echo(const char* buffer) {
    char *json_body = httpc_extract_json_body(buffer);
    if (!json_body) {
        httpc_response_t* error_response = httpc_create_json_error_response(400, "Corpo JSON não encontrado");
        char* response_str = httpc_response_to_string(error_response);
        httpc_free_response(error_response);
        return response_str;
    }
    
    cJSON *json = httpc_parse_json(json_body);
    free(json_body);
    
    if (!json) {
        httpc_response_t* error_response = httpc_create_json_error_response(400, "JSON inválido");
        char* response_str = httpc_response_to_string(error_response);
        httpc_free_response(error_response);
        return response_str;
    }
    
    cJSON *response_json = httpc_json_create_object();
    httpc_json_add_string(response_json, "message", "data successfully received");
    httpc_json_add_string(response_json, "received_data", cJSON_Print(json));
    httpc_json_add_bool(response_json, "success", 1);
    
    httpc_response_t* response = httpc_create_json_response(200, response_json);
    char* response_str = httpc_response_to_string(response);
    
    httpc_json_free(json);
    httpc_json_free(response_json);
    httpc_free_response(response);
    
    return (response_str);
}

int main(void) {
    if (httpc_init() != 0) {
        return 1;
    }
    
    httpc_config_t config = {
        .port = 8080,
        .backlog = 10,
        .max_clients = 10,
        .host = "127.0.0.1"
    };
    
    if (httpc_configure(&config) != 0) {
        httpc_cleanup();
        return 1;
    }
    
    httpc_add_route(&g_router, "GET", "", handle_home);
    httpc_add_route(&g_router, "GET", "status", handle_api_status);
    httpc_add_route(&g_router, "POST", "json/echo", handle_json_echo);
    
    return httpc_start();
}
