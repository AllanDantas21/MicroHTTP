#include "api/httpc.h"
#include "core/error_handling.h"
#include "core/logger.h"
#include <arpa/inet.h>
#include "core/server.h"
#include "structs/connection.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

int allocate_buffer(char **buffer, size_t size) {
    *buffer = malloc(size);
    if (handle_memory_error(*buffer, __func__, __LINE__) == NULL) {
        return 0;
    }
    return 1;
}

static http_status get_response_status(const char* response) {
    if (strstr(response, "HTTP/1.1 200")) return STATUS_200;
    if (strstr(response, "HTTP/1.1 405")) return STATUS_405;
    if (strstr(response, "HTTP/1.1 404")) return STATUS_404;
    if (strstr(response, "HTTP/1.1 400")) return STATUS_400;
    if (strstr(response, "HTTP/1.1 500")) return STATUS_500;
    return STATUS_200;
}

static int set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "fcntl(F_GETFL) failed");
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "fcntl(F_SETFL) O_NONBLOCK failed");
		return -1;
	}
	return 0;
}

static int add_epoll_fd(int epoll_fd, int fd, uint32_t events, void *ptr) {
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "epoll_ctl ADD failed");
		return -1;
	}
	return 0;
}

static int mod_epoll_fd(int epoll_fd, int fd, uint32_t events, void *ptr) {
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = ptr;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "epoll_ctl MOD failed");
		return -1;
	}
	return 0;
}

static void remove_connection(int epoll_fd, connection_t *conn) {
	if (!conn->in_use) return;
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, NULL);
	close(conn->fd);
	conn->fd = -1;
	conn->in_use = 0;
	conn->buffer_len = 0;
	if (conn->response) {
		free(conn->response);
		conn->response = NULL;
	}
	conn->response_len = 0;
	conn->response_sent = 0;
}

static int find_free_slot(connection_t *conns, size_t max) {
	for (size_t i = 0; i < max; ++i) {
		if (!conns[i].in_use) return (int)i;
	}
	return -1;
}

 
static int headers_complete(const char *buf, size_t len) {
	if (len < 4) return 0;
	for (size_t i = 3; i < len; ++i) {
		if (buf[i-3] == '\r' && buf[i-2] == '\n' && buf[i-1] == '\r' && buf[i] == '\n') {
			return 1;
		}
	}
	return 0;
}

void main_handler(int serverSocket) {
	const size_t MAX_CONNECTIONS = (size_t)MAX_CLIENTS * 2;
	connection_t connections[(size_t)MAX_CLIENTS * 2];
	memset(connections, 0, sizeof(connections));

	if (set_nonblocking(serverSocket) == -1) {
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "epoll_create1 failed");
		return;
	}

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.ptr = NULL;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
		debug_error_detailed(__func__, __LINE__, errno, "epoll_ctl ADD server failed");
		close(epoll_fd);
		return;
	}

	struct epoll_event events[64];
	while (httpc_is_running()) {
		int n = epoll_wait(epoll_fd, events, (int)(sizeof(events)/sizeof(events[0])), -1);
		if (n == -1) {
			if (errno == EINTR) {
				continue;
			}
			debug_error_detailed(__func__, __LINE__, errno, "epoll_wait failed");
			break;
		}

		for (int i = 0; i < n; ++i) {
			void *ptr = events[i].data.ptr;
			uint32_t evs = events[i].events;

			if (ptr == NULL) {
				while (1) {
					struct sockaddr_in clientAddress;
					socklen_t clientAddressLength = sizeof(clientAddress);
					int client_fd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
					if (client_fd == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							break;
						}
						if (handle_accept_error(client_fd, __func__, __LINE__) < 0) {
							break;
						}
					}

					if (set_nonblocking(client_fd) == -1) {
						close(client_fd);
						continue;
					}

					int slot = find_free_slot(connections, MAX_CONNECTIONS);
					if (slot == -1) {
						close(client_fd);
						continue;
					}

					connection_t *conn = &connections[slot];
					memset(conn, 0, sizeof(*conn));
					conn->in_use = 1;
					conn->fd = client_fd;
					conn->state = CONNECTION_STATE_RECV;
					conn->buffer_len = 0;
					conn->response = NULL;
					conn->response_len = 0;
					conn->response_sent = 0;
					inet_ntop(AF_INET, &clientAddress.sin_addr, conn->client_ip, INET_ADDRSTRLEN);
					log_connection(conn->client_ip);

					add_epoll_fd(epoll_fd, client_fd, EPOLLIN | EPOLLET, conn);
				}
				continue;
			}

			connection_t *conn = (connection_t *)ptr;
			if (!conn || !conn->in_use) {
				continue;
			}

			if (evs & (EPOLLHUP | EPOLLERR)) {
				remove_connection(epoll_fd, conn);
				continue;
			}

			if (conn->state == CONNECTION_STATE_RECV && (evs & EPOLLIN)) {
				while (1) {
					ssize_t bytes = recv(conn->fd, conn->buffer + conn->buffer_len, sizeof(conn->buffer) - 1 - conn->buffer_len, 0);
					if (bytes > 0) {
						conn->buffer_len += (size_t)bytes;
						conn->buffer[conn->buffer_len] = '\0';
						if (headers_complete(conn->buffer, conn->buffer_len)) {
							char method[16] = {0};
							char path[256] = {0};
							char *response = NULL;

							if (sscanf(conn->buffer, "%15s %255s", method, path) != 2) {
								response = build_response(400, "text/plain", "Bad Request");
							} else {
								const char *route = (path[0] == '/') ? path + 1 : path;
								route_handler h = router_match(&g_router, method, route);
								if (h) {
									response = h(conn->buffer);
								} else {
									response = build_response(404, "text/plain", "Not Found");
								}
							}
							conn->response = response;
							conn->response_len = strlen(response);
							conn->response_sent = 0;

							http_status status = get_response_status(response);
							log_http_request(method, path, conn->client_ip, status);

							conn->state = CONNECTION_STATE_SEND;
							mod_epoll_fd(epoll_fd, conn->fd, EPOLLOUT | EPOLLET, conn);
							break;
						}
						continue;
					} else if (bytes == 0) {
						remove_connection(epoll_fd, conn);
						break;
					} else {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							break;
						}
						debug_error_detailed(__func__, __LINE__, errno, "recv failed");
						remove_connection(epoll_fd, conn);
						break;
					}
				}
			}

			if (conn->state == CONNECTION_STATE_SEND && (evs & EPOLLOUT)) {
				while (conn->response_sent < conn->response_len) {
					ssize_t sent = send(conn->fd, conn->response + conn->response_sent, conn->response_len - conn->response_sent, 0);
					if (sent > 0) {
						conn->response_sent += (size_t)sent;
						continue;
					} else if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
						break;
					} else {
						debug_error_detailed(__func__, __LINE__, errno, "send failed");
						remove_connection(epoll_fd, conn);
						goto next_event;
					}
				}
				if (conn->response_sent >= conn->response_len) {
					remove_connection(epoll_fd, conn);
				}
			}

			next_event: ;
		}
	}
	close(epoll_fd);
}