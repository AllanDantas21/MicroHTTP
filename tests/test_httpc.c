#include "greatest.h"
#include "../includes/api/httpc.h"
#include "../includes/json/json_utils.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Test to verify if initialization works - SHOULD return 0
TEST test_httpc_init(void) {
    int result = httpc_init();
    ASSERT_EQ(0, result);
    PASS();
}

// Test to verify if configuration works - SHOULD return 0
TEST test_httpc_configure(void) {
    errno = 0;
    
    httpc_config_t config;
    memset(&config, 0, sizeof(httpc_config_t));
    config.port = 8080;
    config.backlog = 10;
    config.max_clients = 10;
    config.on_request = NULL;
    config.on_error = NULL;
    
    int result = httpc_configure(&config);
    ASSERT_EQ(0, result);
    PASS();
}

// Test to verify if HTTP response creation works - SHOULD return valid HTTP response string
TEST test_build_response(void) {
    const char* body = "<html><body><h1>Teste</h1></body></html>";
    char* response = build_response(200, "text/html", body);
    
    ASSERT(response != NULL);
    ASSERT(strstr(response, "HTTP/1.1 200 OK") != NULL);
    ASSERT(strstr(response, "Content-Type: text/html") != NULL);
    ASSERT(strstr(response, body) != NULL);
    
    free(response);
    PASS();
}

// Test to verify if JSON response creation works - SHOULD return valid JSON response with 200 status
TEST test_httpc_create_json_response(void) {
    cJSON* json = httpc_json_create_object();
    httpc_json_add_string(json, "message", "teste");
    httpc_json_add_bool(json, "success", 1);
    
    httpc_response_t* response = httpc_create_json_response(200, json);
    ASSERT(response != NULL);
    ASSERT_EQ(200, response->status_code);
    ASSERT(strcmp(response->content_type, "application/json") == 0);
    
    char* response_str = httpc_response_to_string(response);
    ASSERT(response_str != NULL);
    ASSERT(strstr(response_str, "HTTP/1.1 200 OK") != NULL);
    ASSERT(strstr(response_str, "Content-Type: application/json") != NULL);
    ASSERT(strstr(response_str, "teste") != NULL);
    
    free(response_str);
    httpc_json_free(json);
    httpc_free_response(response);
    PASS();
}

// Test to verify if JSON error response creation works - SHOULD return valid JSON error response with 400 status
TEST test_httpc_create_json_error_response(void) {
    httpc_response_t* response = httpc_create_json_error_response(400, "Erro de teste");
    ASSERT(response != NULL);
    ASSERT_EQ(400, response->status_code);
    ASSERT(strcmp(response->content_type, "application/json") == 0);
    
    char* response_str = httpc_response_to_string(response);
    ASSERT(response_str != NULL);
    ASSERT(strstr(response_str, "HTTP/1.1 400 Bad Request") != NULL);
    ASSERT(strstr(response_str, "Erro de teste") != NULL);
    
    free(response_str);
    httpc_free_response(response);
    PASS();
}

// Test to verify if JSON body extraction works - SHOULD return extracted JSON string
TEST test_httpc_extract_json_body(void) {
    const char* request = "POST /test HTTP/1.1\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: 20\r\n"
                         "\r\n"
                         "{\"test\": \"value\"}";
    
    char* json_body = httpc_extract_json_body(request);
    ASSERT(json_body != NULL);
    ASSERT(strcmp(json_body, "{\"test\": \"value\"}") == 0);
    
    free(json_body);
    PASS();
}

// Test to verify if JSON parsing works - SHOULD return parsed JSON object with correct values
TEST test_httpc_parse_json(void) {
    const char* json_string = "{\"name\": \"teste\", \"value\": 42, \"active\": true}";
    cJSON* json = httpc_parse_json(json_string);
    
    ASSERT(json != NULL);
    ASSERT(httpc_json_has_key(json, "name"));
    ASSERT(httpc_json_has_key(json, "value"));
    ASSERT(httpc_json_has_key(json, "active"));
    
    const char* name = httpc_json_get_string(json, "name");
    ASSERT(strcmp(name, "teste") == 0);
    
    double value = httpc_json_get_number(json, "value", 0);
    ASSERT_EQ(42, value);
    
    int active = httpc_json_get_bool(json, "active", 0);
    ASSERT_EQ(1, active);
    
    httpc_json_free(json);
    PASS();
}

// Test to verify if route addition works - SHOULD return 0
TEST test_httpc_add_route(void) {
    httpc_response_t* test_handler(httpc_request_t* req) {
        (void)req;
        return httpc_create_response(200, "text/plain", "Teste");
    }
    
    int result = httpc_add_route(&g_router, "GET", "test", test_handler);
    ASSERT_EQ(0, result);
    PASS();
}

// Test to verify if cleanup works - SHOULD complete without errors
TEST test_httpc_cleanup(void) {
    httpc_cleanup();
    PASS();
}

SUITE(suite_httpc) {
    RUN_TEST(test_httpc_init);
    RUN_TEST(test_httpc_configure);
    RUN_TEST(test_build_response);
    RUN_TEST(test_httpc_create_json_response);
    RUN_TEST(test_httpc_create_json_error_response);
    RUN_TEST(test_httpc_extract_json_body);
    RUN_TEST(test_httpc_parse_json);
    RUN_TEST(test_httpc_add_route);
    RUN_TEST(test_httpc_cleanup);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite_httpc);
    GREATEST_MAIN_END();
}
