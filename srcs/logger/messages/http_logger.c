#include "http.h"

static char* get_current_time(void) {
    static char time_buffer[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return time_buffer;
}

static const char* get_status_text(http_status status) {
    switch (status) {
        case STATUS_200: return "OK";
        case STATUS_400: return "Bad Request";
        case STATUS_404: return "Not Found";
        case STATUS_405: return "Method Not Allowed";
        case STATUS_500: return "Internal Server Error";
        default: return "Unknown";
    }
}

void log_http_request(const char* method, const char* path, const char* client_ip, http_status status_code) {
    printf("[%s] %s %s from %s - %d %s\n",
           get_current_time(),
           method,
           path ? path : "/",
           client_ip,
           status_code,
           get_status_text(status_code));
}

void log_connection(const char* client_ip) {
    printf("[%s] New connection from %s\n",
           get_current_time(),
           client_ip);
}

void log_error(const char* error_msg) {
    printf("[%s] ERROR: %s\n",
           get_current_time(),
           error_msg);
}
