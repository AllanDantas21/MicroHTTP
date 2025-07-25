#include "http.h"
#include "handlers/methods.h"

char* handle_post_request(const char* buffer) {
    static char response_buffer[BUFFER_SIZE];
    
    char *body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4;
    }
    
    if (body && strlen(body) > 0) {
        snprintf(response_buffer, sizeof(response_buffer),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %zu\r\n"
                "\r\n"
                "POST data received: %s", 
                strlen("POST data received: ") + strlen(body), body);
    } else {
        snprintf(response_buffer, sizeof(response_buffer),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 30\r\n"
                "\r\n"
                "POST request received (no data)");
    }
    
    return response_buffer;
}
