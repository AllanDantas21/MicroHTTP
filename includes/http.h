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

#define PORT 8080
#define BACKLOG 10

void welcome_message(void); 
void main_handler(int serverSocket, struct sockaddr_in *clientAddress, socklen_t clientAddressLength);

#endif
