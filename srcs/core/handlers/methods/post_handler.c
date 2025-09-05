#include "api/http.h"
#include "core/handlers/methods.h"
#include "core/router.h"
#include "api/httpc.h"

char* handle_post_request(const char* buffer) {
	static char response_buffer[BUFFER_SIZE];

	char method[16];
	char path[256];
	if (sscanf(buffer, "%15s %255s", method, path) != 2) {
		const char* body = "Bad Request";
		snprintf(response_buffer, sizeof(response_buffer),
			 "HTTP/1.1 400 Bad Request\r\n"
			 "Content-Type: text/plain\r\n"
			 "Content-Length: %zu\r\n"
			 "\r\n"
			 "%s", strlen(body), body);
		return response_buffer;
	}

	const char* route = (path[0] == '/') ? path + 1 : path;

	route_handler h = router_match(&g_router, "POST", route);
	if (h) return h(buffer);

	const char* msg = "Not Found";
	snprintf(response_buffer, sizeof(response_buffer),
		 "HTTP/1.1 404 Not Found\r\n"
		 "Content-Type: text/plain\r\n"
		 "Content-Length: %zu\r\n"
		 "\r\n"
		 "%s", strlen(msg), msg);
	return response_buffer;
}
