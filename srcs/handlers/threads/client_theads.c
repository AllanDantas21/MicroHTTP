#include "http.h"

void create_client_thread(client_info *info) {
    pthread_t thread;
    client_info *client_data = malloc(sizeof(client_info));
    if (client_data == NULL) {
        log_error("Failed to allocate memory for client data");
        close(info->clientSocketFd);
        return;
    }
    
    *client_data = *info;
    pthread_create(&thread, NULL, handle_client, client_data);
    pthread_detach(thread);
}