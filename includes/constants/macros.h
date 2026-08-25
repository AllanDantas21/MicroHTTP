#ifndef HTTPC_MACROS_H
#define HTTPC_MACROS_H

#define HTTPC_GET(server, path, handler) httpc_server_add_route(server, "GET", path, handler)
#define HTTPC_POST(server, path, handler) httpc_server_add_route(server, "POST", path, handler)
#define HTTPC_PUT(server, path, handler) httpc_server_add_route(server, "PUT", path, handler)
#define HTTPC_DELETE(server, path, handler) httpc_server_add_route(server, "DELETE", path, handler)
#define HTTPC_PATCH(server, path, handler) httpc_server_add_route(server, "PATCH", path, handler)

#define HTTPC_OK(response) httpc_create_response(200, "text/plain", response)
#define HTTPC_CREATED(response) httpc_create_response(201, "text/plain", response)
#define HTTPC_NOT_FOUND(response) httpc_create_response(404, "text/plain", response)
#define HTTPC_BAD_REQUEST(response) httpc_create_response(400, "text/plain", response)
#define HTTPC_INTERNAL_ERROR(response) httpc_create_response(500, "text/plain", response)

#endif
