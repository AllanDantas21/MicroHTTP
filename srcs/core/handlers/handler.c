#include "api/httpc.h"
#include "core/error_handling.h"
#include "core/logger.h"
#include "core/request_parser.h"
#include "api/response.h"
#include "core/server.h"
#include "core/server_internal.h"
#include "structs/connection.h"
#include "constants/constants.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return (-1);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return (-1);
    }
    return (0);
}

static int update_epoll(int epoll_fd, int operation, int fd,
                        uint32_t events, void *ptr)
{
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.ptr = ptr;
    if (epoll_ctl(epoll_fd, operation, fd,
                  operation == EPOLL_CTL_DEL ? NULL : &event) == -1) {
        return (-1);
    }
    return (0);
}

static void close_connection(httpc_server_t *server, connection_t *connection)
{
    if (!server || !connection || !connection->in_use) {
        return;
    }
    update_epoll(server->epoll_fd, EPOLL_CTL_DEL, connection->fd, 0, NULL);
    close(connection->fd);
    free(connection->response);
    memset(connection, 0, sizeof(*connection));
    connection->fd = -1;
}

static int find_free_slot(const httpc_server_t *server)
{
    for (size_t index = 0; index < server->connection_count; ++index) {
        if (!server->connections[index].in_use) {
            return ((int)index);
        }
    }
    return (-1);
}

static int accept_connections(httpc_server_t *server)
{
    while (1) {
        struct sockaddr_in client_address;
        socklen_t address_length = sizeof(client_address);
        int client_fd = accept(server->server_socket,
                               (struct sockaddr *)&client_address,
                               &address_length);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return (0);
            }
            return (-1);
        }

        int slot = find_free_slot(server);
        if (slot < 0 || set_nonblocking(client_fd) != 0) {
            close(client_fd);
            continue;
        }

        connection_t *connection = &server->connections[slot];
        memset(connection, 0, sizeof(*connection));
        connection->in_use = 1;
        connection->fd = client_fd;
        connection->state = CONNECTION_STATE_RECV;
        inet_ntop(AF_INET, &client_address.sin_addr, connection->client_ip,
                  INET_ADDRSTRLEN);
        if (update_epoll(server->epoll_fd, EPOLL_CTL_ADD, client_fd,
                         EPOLLIN | EPOLLET, connection) != 0) {
            close_connection(server, connection);
            continue;
        }
        log_connection(connection->client_ip);
    }
}

static int headers_complete(const char *buffer, size_t length)
{
    if (length < 4) {
        return (0);
    }
    for (size_t index = 3; index < length; ++index) {
        if (buffer[index - 3] == '\r' && buffer[index - 2] == '\n' &&
            buffer[index - 1] == '\r' && buffer[index] == '\n') {
            return (1);
        }
    }
    return (0);
}

static int parse_complete_request(connection_t *connection)
{
    return (headers_complete(connection->buffer, connection->buffer_len));
}

static int dispatch_request(httpc_server_t *server, connection_t *connection)
{
    char method[16] = {0};
    char path[256] = {0};
    httpc_request_t request;
    char *response = NULL;

    if (httpc_parse_request(connection->buffer, &request) != 0) {
        response = build_response(400, "text/plain", "Bad Request");
    } else {
        strncpy(method, request.method ? request.method : "", sizeof(method) - 1);
        strncpy(path, request.path ? request.path : "", sizeof(path) - 1);
        const char *route = path[0] == '/' ? path + 1 : path;
        route_handler handler = router_match(&server->router, method, route);
        httpc_response_t *httpc_response = handler ? handler(&request) :
            httpc_create_response(404, "text/plain", "Not Found");
        response = httpc_response ? httpc_response_to_string(httpc_response) :
            build_response(500, "text/plain", "Internal Server Error");
        httpc_free_response(httpc_response);
        httpc_free_request(&request);
    }

    if (!response) {
        response = build_response(500, "text/plain", "Internal Server Error");
    }
    connection->response = response;
    connection->response_len = strlen(response);
    connection->response_sent = 0;
    connection->state = CONNECTION_STATE_SEND;
    return (update_epoll(server->epoll_fd, EPOLL_CTL_MOD, connection->fd,
                          EPOLLOUT | EPOLLET, connection));
}

static int read_connection(httpc_server_t *server, connection_t *connection)
{
    while (1) {
        ssize_t bytes = recv(connection->fd,
                             connection->buffer + connection->buffer_len,
                             sizeof(connection->buffer) - 1 - connection->buffer_len,
                             0);
        if (bytes > 0) {
            connection->buffer_len += (size_t)bytes;
            connection->buffer[connection->buffer_len] = '\0';
            if (connection->buffer_len > MAX_HEADER_SIZE + MAX_BODY_SIZE) {
                close_connection(server, connection);
                return (-1);
            }
            if (parse_complete_request(connection)) {
                return (dispatch_request(server, connection));
            }
        } else if (bytes == 0) {
            close_connection(server, connection);
            return (-1);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return (0);
        } else {
            close_connection(server, connection);
            return (-1);
        }
    }
}

static int write_connection(httpc_server_t *server, connection_t *connection)
{
    while (connection->response_sent < connection->response_len) {
        ssize_t sent = send(connection->fd,
                            connection->response + connection->response_sent,
                            connection->response_len - connection->response_sent,
                            MSG_NOSIGNAL);
        if (sent > 0) {
            connection->response_sent += (size_t)sent;
        } else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return (0);
        } else {
            close_connection(server, connection);
            return (-1);
        }
    }

    free(connection->response);
    connection->response = NULL;
    connection->response_len = 0;
    connection->response_sent = 0;
    connection->buffer_len = 0;
    connection->state = CONNECTION_STATE_RECV;
    return (update_epoll(server->epoll_fd, EPOLL_CTL_MOD, connection->fd,
                         EPOLLIN | EPOLLET, connection));
}

void main_handler(httpc_server_t *server)
{
    if (!server || !server->connections || server->server_socket < 0 ||
        server->epoll_fd < 0 || set_nonblocking(server->server_socket) != 0) {
        return;
    }
    if (update_epoll(server->epoll_fd, EPOLL_CTL_ADD, server->server_socket,
                     EPOLLIN, NULL) != 0) {
        return;
    }

    struct epoll_event events[64];
    while (httpc_server_is_running(server)) {
        int event_count = epoll_wait(server->epoll_fd, events, 64, -1);
        if (event_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return;
        }
        for (int index = 0; index < event_count; ++index) {
            connection_t *connection = events[index].data.ptr;
            uint32_t event_flags = events[index].events;
            if (!connection) {
                accept_connections(server);
                continue;
            }
            if (event_flags & (EPOLLHUP | EPOLLERR)) {
                close_connection(server, connection);
                continue;
            }
            if (connection->state == CONNECTION_STATE_RECV &&
                (event_flags & EPOLLIN)) {
                read_connection(server, connection);
            } else if (connection->state == CONNECTION_STATE_SEND &&
                       (event_flags & EPOLLOUT)) {
                write_connection(server, connection);
            }
        }
    }
}
