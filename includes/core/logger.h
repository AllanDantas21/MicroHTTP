#ifndef LOGGER_H
#define LOGGER_H

#include "structs/enums.h"

void logger_message(logger_status status);
void welcome_message(void);
void server_welcome_message(int port);
void log_http_request(const char* method, const char* path, const char* client_ip, http_status status_code);
void log_connection(const char* client_ip);
void log_error(const char* error_msg);
void log_info(const char* info_msg);
void log_success(const char* success_msg);
int init_logger_system(void);
void cleanup_logger_system(void);

#endif
