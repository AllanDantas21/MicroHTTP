#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "constants/constants.h"
#include "structs/enums.h"
#include "handlers/methods.h"
#include "router.h"

void main_handler(int serverSocket);
void create_client_thread(client_info *info);
void *handle_client(void *arg);

/* Logger funcs */
void logger_message(logger_status status); 
void welcome_message(void);
void log_http_request(const char* method, const char* path, const char* client_ip, http_status status_code);
void log_connection(const char* client_ip);
void log_error(const char* error_msg);

#endif
