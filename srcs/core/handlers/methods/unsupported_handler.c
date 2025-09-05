#include "api/http.h"
#include "core/methods.h"

char* handle_unsupported_method(const char* method) {
    (void)method;
    
    static char response[] = "HTTP/1.1 405 Method Not Allowed\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 19\r\n"
                            "\r\n"
                            "Method not allowed!";
    
    return response;
}
