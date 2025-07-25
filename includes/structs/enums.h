#ifndef ENUMS_H
#define ENUMS_H

#include <netinet/in.h>

typedef enum {
    LOG_WELCOME,
    LOG_REQUEST,
    LOG_ERROR,
    LOG_CONNECTION,
} logger_status;

typedef enum {
    STATUS_200 = 200,
    STATUS_405 = 405,
    STATUS_400 = 400,
    STATUS_404 = 404,
    STATUS_500 = 500,
} http_status;

typedef struct {
    int clientSocketFd;
    char client_ip[INET_ADDRSTRLEN];
} client_info;

#endif