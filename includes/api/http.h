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
#include "core/handlers/methods.h"
#include "core/router.h"
#include "core/logger.h"
#include "core/error_handling.h"

void main_handler(int serverSocket);
void create_client_thread(client_info *info);
void *handle_client(void *arg);

#endif
