#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include "../structs/httpc.h"
#include "../structs/route.h"

int setup_server_address(struct sockaddr_in *serverAddress, int port);
int create_server_socket(void);
int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress);
int start_listening(int serverSocket, int backlog);
httpc_server_t *httpc_server_create(const httpc_config_t *config);
int httpc_server_start(httpc_server_t *server);
int httpc_server_run(httpc_server_t *server);
int httpc_server_stop(httpc_server_t *server);
void httpc_server_destroy(httpc_server_t *server);
int httpc_server_add_route(httpc_server_t *server, const char *method,
						   const char *path, route_handler handler);
int httpc_start(void);
int httpc_stop(void);
int httpc_is_running(void);
int httpc_get_server_socket(void);
int httpc_setup_signals(void);

#endif 