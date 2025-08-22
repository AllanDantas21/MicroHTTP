#include "../../includes/server/server.h"

int setup_server_address(struct sockaddr_in *serverAddress)
{
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(PORT);
    serverAddress->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    return 0;
}

int create_server_socket(void)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    return serverSocket;
}

int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress)
{
    if (bind(serverSocket, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0) {
        printf("Error: The server is not bound to the address.\n");
        return 1;
    }
    return 0;
}

int start_listening(int serverSocket)
{
    if (listen(serverSocket, BACKLOG) < 0) {
        printf("Error: The server is not listening.\n");
        return 1;
    }
    return 0;
} 
