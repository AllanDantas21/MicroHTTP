#include "http.h"

static void handle_client_connection(int clientSocket) {
    (void)clientSocket;
    printf("mocked handler connection to client %d\n", clientSocket);
}

void main_handler(int serverSocket, struct sockaddr_in *clientAddress, socklen_t clientAddressLength) {
    while (42) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)clientAddress, &clientAddressLength);

        if (clientSocket < 0) {
            perror("Error accepting connection");
            continue;
        }

        handle_client_connection(clientSocket);
        close(clientSocket);
    } 
}