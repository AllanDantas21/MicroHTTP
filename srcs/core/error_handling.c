#include "../../includes/core/error_handling.h"
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>

static const char* get_errno_description(int errno_code) {
    switch (errno_code) {
        case EACCES:
            return "Permission denied";
        case EADDRINUSE:
            return "Address already in use";
        case EADDRNOTAVAIL:
            return "Address not available";
        case EAFNOSUPPORT:
            return "Address family not supported";
        case EAGAIN:
            return "Resource temporarily unavailable";
        case EBADF:
            return "Bad file descriptor";
        case ECONNREFUSED:
            return "Connection refused";
        case ECONNRESET:
            return "Connection reset by peer";
        case EINTR:
            return "Interrupted system call";
        case EINVAL:
            return "Invalid argument";
        case EIO:
            return "Input/output error";
        case EISCONN:
            return "Socket is already connected";
        case EMFILE:
            return "Too many open files";
        case EMSGSIZE:
            return "Message too long";
        case ENETDOWN:
            return "Network is down";
        case ENETUNREACH:
            return "Network unreachable";
        case ENFILE:
            return "Too many open files in system";
        case ENOBUFS:
            return "No buffer space available";
        case ENOMEM:
            return "Out of memory";
        case ENOTCONN:
            return "Socket is not connected";
        case ENOTSOCK:
            return "Descriptor is not a socket";
        case EOPNOTSUPP:
            return "Operation not supported";
        case EPIPE:
            return "Broken pipe";
        case EPROTONOSUPPORT:
            return "Protocol not supported";
        case ETIMEDOUT:
            return "Connection timed out";
        default:
            return "Unknown error";
    }
}

void debug_error_detailed(const char *function_name, int line_number, 
                         int errno_code, const char *format, ...) {
    
    const char *errno_desc = get_errno_description(errno_code);
    
    va_list args;
    va_start(args, format);
    
    char custom_msg[512];
    vsnprintf(custom_msg, sizeof(custom_msg), format, args);
    va_end(args);
    
    syslog(LOG_ERR, "[DEBUG] %s:%d - %s (errno: %d - %s)", 
           function_name, line_number, custom_msg, errno_code, errno_desc);
    
    fprintf(stderr, "[DEBUG] %s:%d - %s (errno: %d - %s)\n", 
            function_name, line_number, custom_msg, errno_code, errno_desc);
}

int handle_socket_error(int socket_fd, const char *function_name, int line_number) {
    if (socket_fd < 0) {
        int current_errno = errno;
        debug_error_detailed(function_name, line_number, current_errno,
                           "Failed to create/accept socket");
        return -1;
    }
    return 0;
}

int handle_bind_error(int result, const char *function_name, int line_number) {
    if (result < 0) {
        int current_errno = errno;
        debug_error_detailed(function_name, line_number, current_errno,
                           "Failed to bind socket");
        return -1;
    }
    return 0;
}

int handle_listen_error(int result, const char *function_name, int line_number) {
    if (result < 0) {
        int current_errno = errno;
        debug_error_detailed(function_name, line_number, current_errno,
                           "Failed to listen on socket");
        return -1;
    }
    return 0;
}

int handle_accept_error(int result, const char *function_name, int line_number) {
    if (result < 0) {
        int current_errno = errno;
        if (current_errno != EAGAIN && current_errno != EINTR) {
            debug_error_detailed(function_name, line_number, current_errno,
                               "Failed to accept connection");
        }
        return -1;
    }
    return 0;
}

int handle_io_error(ssize_t result, const char *operation, 
                   const char *function_name, int line_number) {
    if (result < 0) {
        int current_errno = errno;
        if (current_errno != EAGAIN && current_errno != EINTR) {
            debug_error_detailed(function_name, line_number, current_errno,
                               "I/O operation failed: %s", operation);
        }
        return -1;
    } else if (result == 0) {
        debug_error_detailed(function_name, line_number, 0,
                           "Connection closed by peer during: %s", operation);
        return -1;
    }
    return 0;
}

void* handle_memory_error(void *ptr, const char *function_name, int line_number) {
    if (ptr == NULL) {
        debug_error_detailed(function_name, line_number, ENOMEM,
                           "Memory allocation failed");
    }
    return ptr;
}

int handle_thread_error(int result, const char *function_name, int line_number) {
    if (result != 0) {
        debug_error_detailed(function_name, line_number, result,
                           "Thread creation/operation failed");
        return -1;
    }
    return 0;
}

int init_error_handling_system(void) {
    openlog("MicroHTTP-Debug", LOG_PID | LOG_CONS, LOG_USER);
    
    if (errno != 0) {
        fprintf(stderr, "[DEBUG] Could not open syslog: %s\n", 
                strerror(errno));
        errno = 0;
    }
    
    return 0;
}

void cleanup_error_handling_system(void) {
    closelog();
}
