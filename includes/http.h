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

void main_handler(int serverSocket);
void create_client_thread(int clientSocketFd);
void *handle_client(void *arg);

/* Logger funcs */
void logger_message(logger_status status); 
void welcome_message(void);

#endif
