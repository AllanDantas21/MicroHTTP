#include "http.h"
#include <arpa/inet.h>

int allocate_buffer(char **buffer, size_t size) {
    *buffer = malloc(size);
    if (handle_memory_error(*buffer, __func__, __LINE__) == NULL) {
        return 0;
    }
    return 1;
}

static char* extract_path_from_request(const char* buffer) {
    static char path[256];
    char method[16];
    
    if (sscanf(buffer, "%15s %255s", method, path) == 2) {
        return path;
    }
    return "/";
}

static http_status get_response_status(const char* response) {
    if (strstr(response, "HTTP/1.1 200")) return STATUS_200;
    if (strstr(response, "HTTP/1.1 405")) return STATUS_405;
    if (strstr(response, "HTTP/1.1 404")) return STATUS_404;
    if (strstr(response, "HTTP/1.1 400")) return STATUS_400;
    if (strstr(response, "HTTP/1.1 500")) return STATUS_500;
    return STATUS_200;
}

void *handle_client(void *arg) {
    client_info *info = (client_info *)arg;
    int clientSocketFd = info->clientSocketFd;
    char *client_ip = info->client_ip;
    
    char *buffer;
    if (!allocate_buffer(&buffer, BUFFER_SIZE)) {
        close(clientSocketFd);
        free(info);
        return NULL;
    }
    
    ssize_t bytes_received = recv(clientSocketFd, buffer, BUFFER_SIZE - 1, 0);
    if (handle_io_error(bytes_received, "recv", __func__, __LINE__) < 0) {
        close(clientSocketFd);
        free(buffer);
        free(info);
        return NULL;
    }
    buffer[bytes_received] = '\0';
    
    char method[16];
    sscanf(buffer, "%15s", method);
    
    char* path = extract_path_from_request(buffer);
    
    char *response;
    
    if (strcmp(method, "GET") == 0) {
        response = handle_get_request(buffer);
    } else if (strcmp(method, "POST") == 0) {
        response = handle_post_request(buffer);
    } else {
        response = handle_unsupported_method(method);
    }
    
    http_status status = get_response_status(response);
    log_http_request(method, path, client_ip, status);
    
    ssize_t bytes_sent = send(clientSocketFd, response, strlen(response), 0);
    if (handle_io_error(bytes_sent, "send", __func__, __LINE__) < 0) {
        close(clientSocketFd);
        free(buffer);
        free(info);
        return NULL;
    }
    
    close(clientSocketFd);
    free(buffer);
    free(info);
    return NULL;
}

void main_handler(int serverSocket) {
    while (42) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocketFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (handle_accept_error(clientSocketFd, __func__, __LINE__) < 0) {
            continue;
        }

        client_info info;
        info.clientSocketFd = clientSocketFd;
        inet_ntop(AF_INET, &clientAddress.sin_addr, info.client_ip, INET_ADDRSTRLEN);
        
        log_connection(info.client_ip);
        create_client_thread(&info);
    } 
}