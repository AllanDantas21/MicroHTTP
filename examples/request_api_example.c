#include "../includes/api/httpc.h"
#include "../includes/json/json_utils.h"
#include "../includes/core/request_parser.h"
#include "../srcs/json/cJSON.h"
#include <stdio.h>
#include <string.h>

httpc_response_t* handle_home(httpc_request_t* req) {
    (void)req;
    const char* html = "<html><body><h1>HTTP.c - Request API Example</h1>"
                       "<p>Este exemplo demonstra o uso das novas funções helper para acessar dados da requisição</p>"
                       "<ul>"
                       "<li><a href=\"/query?name=João&age=30&city=São%20Paulo\">Query Params: /query?name=João&age=30&city=São Paulo</a></li>"
                       "<li><a href=\"/headers\">Headers: /headers</a></li>"
                       "<li><a href=\"/info\">Request Info: /info</a></li>"
                       "</ul>"
                       "</body></html>";
    return httpc_html(200, html);
}

httpc_response_t* handle_query_params(httpc_request_t* req) {
    const char* name = httpc_get_query_param(req, "name");
    const char* age = httpc_get_query_param(req, "age");
    const char* city = httpc_get_query_param(req, "city");
    
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "message", "Query parameters extraídos");
    
    cJSON* params = httpc_json_create_object();
    if (name) {
        httpc_json_add_string(params, "name", name);
    }
    if (age) {
        httpc_json_add_string(params, "age", age);
    }
    if (city) {
        httpc_json_add_string(params, "city", city);
    }
    cJSON_AddItemToObject(json, "query_params", params);
    
    return httpc_json_ok(json);
}

httpc_response_t* handle_headers(httpc_request_t* req) {
    const char* user_agent = httpc_get_header(req, "User-Agent");
    const char* content_type = httpc_get_header(req, "Content-Type");
    const char* accept = httpc_get_header(req, "Accept");
    const char* authorization = httpc_get_header(req, "Authorization");
    
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "message", "Headers extraídos");
    
    cJSON* headers = httpc_json_create_object();
    if (user_agent) {
        httpc_json_add_string(headers, "User-Agent", user_agent);
    }
    if (content_type) {
        httpc_json_add_string(headers, "Content-Type", content_type);
    }
    if (accept) {
        httpc_json_add_string(headers, "Accept", accept);
    }
    if (authorization) {
        httpc_json_add_string(headers, "Authorization", "***");
    }
    cJSON_AddItemToObject(json, "headers", headers);
    
    return httpc_json_ok(json);
}

httpc_response_t* handle_request_info(httpc_request_t* req) {
    const char* method = httpc_get_method(req);
    const char* path = httpc_get_path(req);
    const char* body = httpc_get_body(req);
    
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "method", method ? method : "");
    httpc_json_add_string(json, "path", path ? path : "");
    
    if (body) {
        httpc_json_add_string(json, "body", body);
        httpc_json_add_number(json, "body_length", strlen(body));
    } else {
        httpc_json_add_string(json, "body", "");
        httpc_json_add_number(json, "body_length", 0);
    }
    
    return httpc_json_ok(json);
}

httpc_response_t* handle_json_body(httpc_request_t* req) {
    cJSON* json = httpc_parse_json_body(req);
    if (!json) {
        return httpc_json_error(400, "JSON inválido ou corpo não encontrado");
    }
    
    const char* name = httpc_json_get_string(json, "name");
    double age = httpc_json_get_number(json, "age", 0);
    int active = httpc_json_get_bool(json, "active", 0);
    
    cJSON* response_json = httpc_json_create_object();
    httpc_json_add_string(response_json, "message", "JSON parseado com sucesso");
    
    cJSON* data = httpc_json_create_object();
    if (name) {
        httpc_json_add_string(data, "name", name);
    }
    httpc_json_add_number(data, "age", age);
    httpc_json_add_bool(data, "active", active);
    cJSON_AddItemToObject(response_json, "data", data);
    
    httpc_json_free(json);
    return httpc_json_ok(response_json);
}

httpc_response_t* handle_combined_example(httpc_request_t* req) {
    const char* method = httpc_get_method(req);
    const char* path = httpc_get_path(req);
    const char* token = httpc_get_header(req, "Authorization");
    const char* page = httpc_get_query_param(req, "page");
    const char* limit = httpc_get_query_param(req, "limit");
    
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "method", method ? method : "");
    httpc_json_add_string(json, "path", path ? path : "");
    
    if (token) {
        httpc_json_add_string(json, "authenticated", "true");
    } else {
        httpc_json_add_string(json, "authenticated", "false");
    }
    
    cJSON* query = httpc_json_create_object();
    if (page) {
        httpc_json_add_string(query, "page", page);
    }
    if (limit) {
        httpc_json_add_string(query, "limit", limit);
    }
    cJSON_AddItemToObject(json, "query_params", query);
    
    cJSON* body_json = httpc_parse_json_body(req);
    if (body_json) {
        cJSON_AddItemToObject(json, "body_data", body_json);
    }
    
    return httpc_json_ok(json);
}

httpc_response_t* handle_validation_example(httpc_request_t* req) {
    const char* username = httpc_get_query_param(req, "username");
    const char* password = httpc_get_header(req, "X-Password");
    
    if (!username) {
        return httpc_json_error(400, "Parâmetro 'username' é obrigatório");
    }
    
    if (!password) {
        return httpc_json_error(401, "Header 'X-Password' é obrigatório");
    }
    
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "message", "Validação bem-sucedida");
    httpc_json_add_string(json, "username", username);
    httpc_json_add_bool(json, "authenticated", 1);
    
    return httpc_json_ok(json);
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
    httpc_add_route(&g_router, "GET", "query", handle_query_params);
    httpc_add_route(&g_router, "GET", "headers", handle_headers);
    httpc_add_route(&g_router, "GET", "info", handle_request_info);
    httpc_add_route(&g_router, "POST", "json", handle_json_body);
    httpc_add_route(&g_router, "POST", "combined", handle_combined_example);
    httpc_add_route(&g_router, "GET", "validate", handle_validation_example);
    
    return httpc_start();
}

