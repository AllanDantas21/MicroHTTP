#include "http.h"
#include "handlers/methods.h"

char* handle_get_request(const char* buffer) {
    (void)buffer;
    
    static char response[] = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 21\r\n"
                            "\r\n"
                            "GET request received!";
    
    return response;
}
