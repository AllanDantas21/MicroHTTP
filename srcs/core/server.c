#include "../../includes/core/server.h"
#include "../../includes/core/server_internal.h"
#include "../../includes/api/httpc.h"
#include "../../includes/core/error_handling.h"
#include "../../includes/core/logger.h"
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

static httpc_server_t *g_default_server = NULL;
static httpc_server_t *g_signal_server = NULL;

static int validate_server_config(const httpc_config_t *config)
{
    if (!config || config->port <= 0 || config->port > 65535 ||
        config->backlog <= 0 || config->max_clients <= 0) {
        errno = EINVAL;
        return (-1);
    }
    return (0);
}

int setup_server_address(struct sockaddr_in *serverAddress, int port)
{
    if (!serverAddress || port <= 0 || port > 65535) {
        errno = EINVAL;
        return (-1);
    }
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(port);
    serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
    return (0);
}

int create_server_socket(void)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (handle_socket_error(serverSocket, __func__, __LINE__) < 0) {
        return (-1);
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   &(int){1}, sizeof(int)) < 0) {
        close(serverSocket);
        return (-1);
    }
    return (serverSocket);
}

int bind_server_socket(int serverSocket, struct sockaddr_in *serverAddress)
{
    int bind_result = bind(serverSocket, (struct sockaddr *)serverAddress,
                           sizeof(*serverAddress));
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

static void signal_handler(int sig)
{
    if ((sig == SIGINT || sig == SIGTERM) && g_signal_server) {
        httpc_server_stop(g_signal_server);
    }
}

httpc_server_t *httpc_server_create(const httpc_config_t *config)
{
    if (validate_server_config(config) != 0) {
        return (NULL);
    }

    httpc_server_t *server = calloc(1, sizeof(*server));
    if (!server) {
        errno = ENOMEM;
        return (NULL);
    }

    server->config = *config;
    server->server_socket = -1;
    server->epoll_fd = -1;
    server->state = HTTPC_SERVER_CREATED;
    return (server);
}

int httpc_server_start(httpc_server_t *server)
{
    struct sockaddr_in serverAddress;

    if (!server || server->state != HTTPC_SERVER_CREATED) {
        errno = EALREADY;
        return (-1);
    }
    if (setup_server_address(&serverAddress, server->config.port) != 0) {
        return (-1);
    }

    server->server_socket = create_server_socket();
    if (server->server_socket < 0 ||
        bind_server_socket(server->server_socket, &serverAddress) != 0 ||
        start_listening(server->server_socket, server->config.backlog) != 0) {
        if (server->server_socket >= 0) {
            close(server->server_socket);
            server->server_socket = -1;
        }
        return (-1);
    }

    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd < 0) {
        close(server->server_socket);
        server->server_socket = -1;
        return (-1);
    }

    server->connection_count = (size_t)server->config.max_clients;
    server->connections = calloc(server->connection_count,
                                  sizeof(*server->connections));
    if (!server->connections) {
        close(server->epoll_fd);
        close(server->server_socket);
        server->epoll_fd = -1;
        server->server_socket = -1;
        errno = ENOMEM;
        return (-1);
    }

    g_signal_server = server;
    server->state = HTTPC_SERVER_STARTED;
    return (0);
}

int httpc_server_run(httpc_server_t *server)
{
    if (!server || server->state != HTTPC_SERVER_STARTED) {
        errno = EINVAL;
        return (-1);
    }

    server->state = HTTPC_SERVER_RUNNING;
    if (server->config.on_request) {
        server->config.on_request("SERVER", "STARTED", "Server is running");
    }
    main_handler(server);
    if (server->state == HTTPC_SERVER_RUNNING) {
        server->state = HTTPC_SERVER_STOPPED;
    }
    return (0);
}

int httpc_server_stop(httpc_server_t *server)
{
    if (!server) {
        errno = EINVAL;
        return (-1);
    }
    if (server->state == HTTPC_SERVER_STOPPED ||
        server->state == HTTPC_SERVER_CREATED) {
        return (0);
    }

    server->state = HTTPC_SERVER_STOPPING;
    if (g_signal_server == server) {
        g_signal_server = NULL;
    }
    if (server->server_socket >= 0) {
        close(server->server_socket);
        server->server_socket = -1;
    }
    return (0);
}

void httpc_server_destroy(httpc_server_t *server)
{
    if (!server) {
        return;
    }
    httpc_server_stop(server);
    if (server->epoll_fd >= 0) {
        close(server->epoll_fd);
    }
    free(server->connections);
    router_cleanup(&server->router);
    free(server);
}

int httpc_server_add_route(httpc_server_t *server, const char *method,
                           const char *path, route_handler handler)
{
    if (!server) {
        errno = EINVAL;
        return (-1);
    }
    router_add(&server->router, method, path, handler);
    return (0);
}

int httpc_server_is_running(const httpc_server_t *server)
{
    return (server && server->state == HTTPC_SERVER_RUNNING);
}

httpc_server_t *httpc_get_default_server(void)
{
    return (g_default_server);
}

void httpc_destroy_default_server(void)
{
    if (g_default_server) {
        httpc_server_destroy(g_default_server);
        g_default_server = NULL;
    }
}

int httpc_setup_signals(void)
{
    if (signal(SIGINT, signal_handler) == SIG_ERR ||
        signal(SIGTERM, signal_handler) == SIG_ERR) {
        debug_error_detailed(__func__, __LINE__, errno,
                             "Failed to configure signal handlers");
        return (-1);
    }
    return (0);
}

int httpc_start(void)
{
    const httpc_config_t *config = httpc_get_config();

    if (!g_default_server) {
        g_default_server = httpc_server_create(config);
        if (!g_default_server) {
            return (-1);
        }
    }
    if (httpc_server_start(g_default_server) != 0) {
        return (-1);
    }
    return (httpc_server_run(g_default_server));
}

int httpc_stop(void)
{
    return (g_default_server ? httpc_server_stop(g_default_server) : 0);
}

int httpc_is_running(void)
{
    return (httpc_server_is_running(g_default_server));
}

int httpc_get_server_socket(void)
{
    return (g_default_server ? g_default_server->server_socket : -1);
}

int httpc_run(void)
{
    return (g_default_server ? httpc_server_run(g_default_server) : -1);
}
