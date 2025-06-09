#include "http.h"

void *handle_client(void *arg) {
    int clientSocketFd = (int)(intptr_t)arg;
    char *buffer = malloc(BUFFER_SIZE);
    
    recv(clientSocketFd, buffer, BUFFER_SIZE, 0);
    printf("buffer: %s\n", buffer);
    
    close(clientSocketFd);
    free(buffer);
    return NULL;
}

void create_client_thread(int clientSocketFd) {
    pthread_t thread;
    pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)clientSocketFd);
    pthread_detach(thread);
}

void main_handler(int serverSocket) {
    while (42) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocketFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocketFd < 0) {
            perror("Error accepting connection");
            continue;
        }

        create_client_thread(clientSocketFd);
    } 
}