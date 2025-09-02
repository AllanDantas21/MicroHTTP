#include "../../includes/core/utils.h"
#include "../../includes/error_handling.h"
#include <string.h>
#include <stdlib.h>

char* string_dup(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char* dup = malloc(len + 1);
    if (handle_memory_error(dup, __func__, __LINE__) == NULL) return NULL;
    
    strcpy(dup, str);
    return dup;
}

const char* get_status_text(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}
