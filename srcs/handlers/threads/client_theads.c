#include "http.h"

void create_client_thread(int clientSocketFd) {
    pthread_t thread;
    pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)clientSocketFd);
    pthread_detach(thread);
}