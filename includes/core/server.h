#ifndef SERVER_H
#define SERVER_H

#include "api/httpc.h"

int setup_server_address(struct sockaddr_in *serverAddress, int port);
int create_server_socket(void);
int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress);
int start_listening(int serverSocket, int backlog);
int httpc_start(void);
int httpc_stop(void);
int httpc_is_running(void);
int httpc_get_server_socket(void);
int httpc_setup_signals(void);

#endif 