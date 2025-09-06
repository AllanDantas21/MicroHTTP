#ifndef CONNECTION_STRUCTS_H
#define CONNECTION_STRUCTS_H

#include <netinet/in.h>
#include "constants/constants.h"

typedef enum {
	CONNECTION_STATE_RECV = 0,
	CONNECTION_STATE_SEND = 1
} connection_state_t;

typedef struct {
	int in_use;
	int fd;
	connection_state_t state;
	char buffer[BUFFER_SIZE];
	size_t buffer_len;
	char *response;
	size_t response_len;
	size_t response_sent;
	char client_ip[INET_ADDRSTRLEN];
} connection_t;

#endif

