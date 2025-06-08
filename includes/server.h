#ifndef SERVER_H
#define SERVER_H

#include "http.h"

int setup_server_address(struct sockaddr_in *serverAddress);
int create_server_socket(void);
int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress);
int start_listening(int serverSocket);

#endif 