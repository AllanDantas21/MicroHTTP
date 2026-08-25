#ifndef HTTPC_SERVER_INTERNAL_H
#define HTTPC_SERVER_INTERNAL_H

#include <stddef.h>
#include <sys/epoll.h>
#include "../structs/httpc.h"
#include "../structs/route.h"
#include "../structs/connection.h"

typedef enum {
    HTTPC_SERVER_CREATED = 0,
    HTTPC_SERVER_STARTED,
    HTTPC_SERVER_RUNNING,
    HTTPC_SERVER_STOPPING,
    HTTPC_SERVER_STOPPED
} httpc_server_state_t;

struct httpc_server {
    httpc_config_t config;
    int server_socket;
    int epoll_fd;
    httpc_server_state_t state;
    connection_t *connections;
    size_t connection_count;
    Router router;
};

int httpc_server_is_running(const httpc_server_t *server);
void main_handler(httpc_server_t *server);
httpc_server_t *httpc_get_default_server(void);
void httpc_destroy_default_server(void);

#endif
