#include "http.h"
#include "routes.h"

static char* build_plain_response(const char* status_line, const char* body) {
	static char response_buffer[BUFFER_SIZE];
	snprintf(response_buffer, sizeof(response_buffer),
		"%s\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: %zu\r\n"
		"\r\n"
		"%s",
		status_line, strlen(body), body);
	return response_buffer;
}

char* route_get_root(const char* buffer) {
	(void)buffer;
	return build_plain_response("HTTP/1.1 200 OK", "Hello from GET /");
}

char* route_get_health(const char* buffer) {
	(void)buffer;
	return build_plain_response("HTTP/1.1 200 OK", "OK");
}

char* route_post_echo(const char* buffer) {
	static char response_buffer[BUFFER_SIZE];

	char *body = strstr(buffer, "\r\n\r\n");
	if (body) body += 4;

	if (body && strlen(body) > 0) {
		const char* prefix = "POST data received: ";
		size_t body_len = strlen(prefix) + strlen(body);
		snprintf(response_buffer, sizeof(response_buffer),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: %zu\r\n"
			"\r\n"
			"%s%s", body_len, prefix, body);
	} else {
		const char* msg = "POST request received (no data)";
		snprintf(response_buffer, sizeof(response_buffer),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: %zu\r\n"
			"\r\n"
			"%s", strlen(msg), msg);
	}
	return response_buffer;
}
