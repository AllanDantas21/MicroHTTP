#include "http.h"

int allocate_buffer(char **buffer, size_t size) {
    *buffer = malloc(size);
    if (*buffer == NULL) {
        perror("Failed to allocate memory for buffer");
        return 0;
    }
    return -1;
}

void *handle_client(void *arg) {
    int clientSocketFd = (int)(intptr_t)arg;
    char *buffer;
    if (!allocate_buffer(&buffer, BUFFER_SIZE)) {
        close(clientSocketFd);
        return NULL;
    }
    
    recv(clientSocketFd, buffer, BUFFER_SIZE, 0);
    const char *response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: 11\r\n"
                          "\r\n"
                          "Hello World!";
    send(clientSocketFd, response, strlen(response), 0);
    
    close(clientSocketFd);
    free(buffer);
    return NULL;
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