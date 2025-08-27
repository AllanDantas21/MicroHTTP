#ifndef HTTPC_MACROS_H
#define HTTPC_MACROS_H

#define HTTPC_GET(path, handler) httpc_add_route("GET", path, handler)
#define HTTPC_POST(path, handler) httpc_add_route("POST", path, handler)
#define HTTPC_PUT(path, handler) httpc_add_route("PUT", path, handler)
#define HTTPC_DELETE(path, handler) httpc_add_route("DELETE", path, handler)
#define HTTPC_PATCH(path, handler) httpc_add_route("PATCH", path, handler)

#define HTTPC_OK(response) httpc_create_response(200, "text/plain", response)
#define HTTPC_CREATED(response) httpc_create_response(201, "text/plain", response)
#define HTTPC_NOT_FOUND(response) httpc_create_response(404, "text/plain", response)
#define HTTPC_BAD_REQUEST(response) httpc_create_response(400, "text/plain", response)
#define HTTPC_INTERNAL_ERROR(response) httpc_create_response(500, "text/plain", response)

#endif
