#include "../../includes/core/server.h"
#include "../../includes/api/httpc.h"
#include "../../includes/core/methods.h"
#include "../../includes/core/config.h"
#include "../../includes/core/error_handling.h"
#include "../../includes/core/logger.h"
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

static int g_server_socket = -1;
static int g_running = 0;

int setup_server_address(struct sockaddr_in *serverAddress, int port)
{
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(port);
    serverAddress->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    return (0);
}

int create_server_socket(void)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (handle_socket_error(serverSocket, __func__, __LINE__) < 0) {
        return -1;
    }
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    return (serverSocket);
}

int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress)
{
    int bind_result = bind(serverSocket, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
    if (handle_bind_error(bind_result, __func__, __LINE__) < 0) {
        return (-1);
    }
    return (0);
}

int start_listening(int serverSocket, int backlog)
{
    int listen_result = listen(serverSocket, backlog);
    if (handle_listen_error(listen_result, __func__, __LINE__) < 0) {
        return (-1);
    }
    return (0);
}

static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        const httpc_config_t* config = httpc_get_config();
        if (config && config->on_request) {
            config->on_request("SERVER", "STOPPING", "Received interrupt signal");
        }
        
        g_running = 0;
        
        if (g_server_socket != -1) {
            close(g_server_socket);
            g_server_socket = -1;
        }
    }
}

static void* server_main_loop(void* arg) {
    (void)arg;
    const httpc_config_t* config = httpc_get_config();
    if (config && config->on_request) {
        config->on_request("SERVER", "STARTED", "Server is running");
    }
    main_handler(g_server_socket);
    return (NULL);
}

int httpc_start(void) {
    struct sockaddr_in serverAddress;
    const httpc_config_t* config = httpc_get_config();
    
    if (g_running) {
        errno = EALREADY;
        return -1;
    }
    
    if (setup_server_address(&serverAddress, config->port) != 0) {
        if (config->on_error) {
            config->on_error("Failed to setup server address");
        }
        return (-1);
    }
    
    g_server_socket = create_server_socket();
    if (g_server_socket < 0) {
        return (-1);
    }
    
    if (bind_server_socket(g_server_socket, &serverAddress) != 0) {
        close(g_server_socket);
        g_server_socket = -1;
        return (-1);
    }
    
    if (start_listening(g_server_socket, config->backlog) != 0) {
        close(g_server_socket);
        g_server_socket = -1;
        return (-1);
    }
    
    g_running = 1;

    if (httpc_setup_signals() != 0) {
        if (config->on_error) {
            config->on_error("Failed to setup signal handlers");
        }
        httpc_stop();
        return (-1);
    }
    
    if (config && config->on_request) {
        config->on_request("SERVER", "RUNNING", "Server loop started");
    }
    
    server_welcome_message(config->host, config->port);
    
    server_main_loop(NULL);
    
    if (config && config->on_request) {
        config->on_request("SERVER", "STOPPED", "Server loop ended");
    }
    
    httpc_stop();
    log_success("Server stopped successfully");

    httpc_cleanup();
    return (0);
}


int httpc_stop(void) {
    if (!g_running) {
        return (0);
    }
    
    g_running = 0;
    
    if (g_server_socket != -1) {
        close(g_server_socket);
        g_server_socket = -1;
    }
    
    
    printf("HTTP.c server stopped\n");
    return (0);
}

int httpc_is_running(void) {
    return (g_running);
}

int httpc_get_server_socket(void) {
    return (g_server_socket);
}

int httpc_setup_signals(void) {
    __sighandler_t sigint_result = signal(SIGINT, signal_handler);
    if (sigint_result == SIG_ERR) { 
        debug_error_detailed(__func__, __LINE__, errno, "Failed to configure handler for SIGINT");
        return (-1);
    }
    
    __sighandler_t sigterm_result = signal(SIGTERM, signal_handler);
    if (sigterm_result == SIG_ERR) { 
        debug_error_detailed(__func__, __LINE__, errno, "Failed to configure handler for SIGTERM");
        return (-1);
    }
    
    return (0);
}

int httpc_run(void) {
    const httpc_config_t* config = httpc_get_config();
    
    if (!g_running) {
        if (config && config->on_error) {
            config->on_error("Server is not running. Call httpc_start() first.");
        }
        return (-1);
    }
    
    if (httpc_setup_signals() != 0) {
        if (config && config->on_error) {
            config->on_error("Failed to setup signal handlers");
        }
        return (-1);
    }
    
    if (config && config->on_request) {
        config->on_request("SERVER", "RUNNING", "Server loop started");
    }
    
    server_welcome_message(config->host, config->port);
    
    while (g_running) {
        sleep(1);
    }
    
    if (config && config->on_request) {
        config->on_request("SERVER", "STOPPED", "Server loop ended");
    }
    
    httpc_cleanup();
    log_success("Server stopped successfully");
    
    return (0);
}
