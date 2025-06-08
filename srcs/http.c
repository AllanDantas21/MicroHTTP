#include "http.h"

static int setup_server_address(struct sockaddr_in *serverAddress);
static int create_server_socket(void);
static int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress);
static int start_listening(int serverSocket);

int main(void)
{
    struct sockaddr_in serverAddress;
    int serverSocket;
    
    setup_server_address(&serverAddress);
    serverSocket = create_server_socket();
    
    if (bind_server_socket(serverSocket, &serverAddress) != 0) return 1;
    if (start_listening(serverSocket) != 0) return 1;

    welcome_message();

    return 0;
}


static int setup_server_address(struct sockaddr_in *serverAddress)
{
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(PORT);
    serverAddress->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    return 0;
}

static int create_server_socket(void)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    return serverSocket;
}

static int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress)
{
    if (bind(serverSocket, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0) {
        printf("Error: The server is not bound to the address.\n");
        return 1;
    }
    return 0;
}

static int start_listening(int serverSocket)
{
    if (listen(serverSocket, BACKLOG) < 0) {
        printf("Error: The server is not listening.\n");
        return 1;
    }
    return 0;
}
