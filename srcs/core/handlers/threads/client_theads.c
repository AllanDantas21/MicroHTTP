#include "api/http.h"

void create_client_thread(client_info *info) {
    pthread_t thread;
    client_info *client_data = malloc(sizeof(client_info));
    if (handle_memory_error(client_data, __func__, __LINE__) == NULL) {
        close(info->clientSocketFd);
        return;
    }
    
    *client_data = *info;
    int thread_result = pthread_create(&thread, NULL, handle_client, client_data);
    if (handle_thread_error(thread_result, __func__, __LINE__) < 0) {
        free(client_data);
        close(info->clientSocketFd);
        return;
    }
    pthread_detach(thread);
}