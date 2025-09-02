#include "../../includes/logger.h"
#include "../../includes/structs/enums.h"
#include "../../includes/constants/constants.h"
#include <stdio.h>
#include <time.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

void welcome_message(void) {
    printf("--------------------------------\n");
    printf("Server is running on port %d\n", PORT);
    printf("Waiting for connections...\n");
    printf("--------------------------------\n\n");
}

void server_welcome_message(const char* host, int port) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] INFO: Server running on http://%s:%d\n", time_str, host, port);
    printf("[%s] INFO: Press Ctrl+C to stop\n", time_str);
    syslog(LOG_INFO, "Server running on http://%s:%d", host, port);
    syslog(LOG_INFO, "Press Ctrl+C to stop");
}

void log_http_request(const char* method, const char* path, const char* client_ip, http_status status_code) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    const char* status_text = (status_code >= 200 && status_code < 300) ? "OK" : 
                             (status_code >= 400) ? "ERROR" : "WARNING";
    
    printf("[%s] %s %s %s %s %d\n", 
           time_str, client_ip, method, path, status_text, status_code);
    syslog(LOG_INFO, "%s %s %s %d", client_ip, method, path, status_code);
}

void log_connection(const char* client_ip) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] New connection from: %s\n", time_str, client_ip);
    syslog(LOG_INFO, "New connection from: %s", client_ip);
}

void log_error(const char* error_msg) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] ERROR: %s\n", time_str, error_msg);
    syslog(LOG_ERR, "ERROR: %s", error_msg);
}

void log_info(const char* info_msg) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] INFO: %s\n", time_str, info_msg);
    syslog(LOG_INFO, "INFO: %s", info_msg);
}

void log_success(const char* success_msg) {
    time_t now = time(0);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    printf("[%s] SUCCESS: %s\n", time_str, success_msg);
    syslog(LOG_INFO, "SUCCESS: %s", success_msg);
}

void logger_message(logger_status status) {
    switch (status) {
        case LOG_WELCOME:
            welcome_message();
            break;
        case LOG_REQUEST:
            /* todo: implemented in log_http_request */
            break;
        case LOG_ERROR:
            /* todo: implemented in log_error */
            break;
        case LOG_CONNECTION:
            /* todo: implemented in log_connection */
            break;
        default:
            log_error("Invalid logger status");
            break;
    }
}

int init_logger_system(void) {
    openlog("MicroHTTP", LOG_PID | LOG_CONS, LOG_USER);
    
    if (errno != 0) {
        printf("WARNING: Could not open syslog: %s\n", 
               strerror(errno));
        errno = 0;
    }
    
    return 0;
}

void cleanup_logger_system(void) {
    closelog();
}
