#include "../includes/api/httpc.h"
#include "../includes/json/json_utils.h"
#include <stdio.h>

httpc_response_t* handle_home(httpc_request_t* req) {
    (void)req;
    const char* html = "<html><body><h1>Bem-vindo ao HTTP.c!</h1><p>Um micro framework HTTP em C</p></body></html>";
    return httpc_html(200, html);
}

httpc_response_t* handle_api_status(httpc_request_t* req) {
    (void)req;
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "status", "running");
    return httpc_json_ok(json);
}

httpc_response_t* handle_json_echo(httpc_request_t* req) {
    if (!req || !req->body) {
        return httpc_json_error(400, "Corpo JSON não encontrado");
    }
    
    cJSON* json = httpc_parse_json(req->body);
    if (!json) {
        return httpc_json_error(400, "JSON inválido");
    }
    
    cJSON* response_json = httpc_json_create_object();
    httpc_json_add_string(response_json, "message", "data successfully received");
    
    char* json_string = cJSON_Print(json);
    if (json_string) {
        httpc_json_add_string(response_json, "received_data", json_string);
        free(json_string);
    }
    
    httpc_json_add_bool(response_json, "success", 1);
    httpc_json_free(json);
    return (httpc_json_ok(response_json));
}

int main(void) {
    if (httpc_init() != 0) {
        return 1;
    }
    
    httpc_config_t config = {
        .port = 8080,
        .backlog = 10,
        .max_clients = 10
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
