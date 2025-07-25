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
    
    int bytes_received = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(clientSocketFd);
        free(buffer);
        return NULL;
    }
    buffer[bytes_received] = '\0';
    
    char method[16];
    sscanf(buffer, "%15s", method);
    
    char *response;
    
    if (strcmp(method, "GET") == 0) {
        response = handle_get_request(buffer);
    } else if (strcmp(method, "POST") == 0) {
        response = handle_post_request(buffer);
    } else {
        response = handle_unsupported_method(method);
    }
    
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