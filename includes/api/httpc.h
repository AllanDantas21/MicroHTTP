/**
 * @file httpc.h
 * @brief HTTP.c - Micro HTTP Framework
 * @version 1.0.0
 * @author Allan Dantas
 * 
 */

#ifndef HTTPC_H
#define HTTPC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "constants/constants.h"
#include "constants/macros.h"
#include "structs/enums.h"
#include "structs/httpc.h"
#include "core/handlers/methods.h"
#include "core/router.h"
#include "core/server/server.h"
#include "json/json_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Router g_router;

/**
 * @brief Initialize the HTTP.c library
 * @return 0 on success, -1 on error
 */
int httpc_init(void);

/**
 * @brief Configure the server with the specified options
 * @param config Server configuration
 * @return 0 on success, -1 on error
 */
int httpc_configure(const httpc_config_t* config);

/**
 * @brief Add a route to the server
 * @param router Pointer to the router instance
 * @param method HTTP method (GET, POST, PUT, DELETE, etc.)
 * @param path Route path
 * @param handler Function that processes the request
 * @return 0 on success, -1 on error
 */
int httpc_add_route(Router* router, const char* method, const char* path, route_handler handler);

/**
 * @brief Start the HTTP server
 * @return 0 on success, -1 on error
 */
int httpc_start(void);

/**
 * @brief Stop the HTTP server
 * @return 0 on success, -1 on error
 */
int httpc_stop(void);

/**
 * @brief Run the server loop until a stop signal is received
 * @return 0 on success, -1 on error
 */
int httpc_run(void);

/**
 * @brief Clean up library resources
 */
void httpc_cleanup(void);

/**
 * @brief Create an HTTP response
 * @param status_code HTTP status code
 * @param content_type Content type
 * @param body Response body
 * @return HTTP response structure
 */
httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body);

/**
 * @brief Free an HTTP response
 * @param response Response to be freed
 */
void httpc_free_response(httpc_response_t* response);

/**
 * @brief Convert an HTTP response to a string
 * @param response HTTP response
 * @return Formatted response string
 */
char* httpc_response_to_string(const httpc_response_t* response);

/**
 * @brief Set a custom header in the response
 * @param response HTTP response
 * @param key Header key
 * @param value Header value
 */
void httpc_set_header(httpc_response_t* response, const char* key, const char* value);

/**
 * @brief Build only the HTTP headers for a response
 */
char* httpc_build_headers(int status_code, const char* content_type, size_t content_length, const char* extra_headers);

/**
 * @brief Build the full response (headers + body)
 */
char* build_response(int status_code, const char* content_type, const char* body);

/**
 * @brief Check if the server is running
 * @return 1 if running, 0 otherwise
 */
int httpc_is_running(void);

/**
 * @brief Get the current server configuration
 * @return Pointer to the current configuration
 */
const httpc_config_t* httpc_get_config(void);

/**
 * @brief Get the server socket
 * @return Socket descriptor or -1 if not active
 */
int httpc_get_server_socket(void);



#ifdef __cplusplus
}
#endif

#endif
