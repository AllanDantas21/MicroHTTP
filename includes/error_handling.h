#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>

void debug_error_detailed(const char *function_name, int line_number, 
                         int errno_code, const char *format, ...);

int handle_socket_error(int socket_fd, const char *function_name, int line_number);
int handle_bind_error(int result, const char *function_name, int line_number);
int handle_listen_error(int result, const char *function_name, int line_number);
int handle_accept_error(int result, const char *function_name, int line_number);
int handle_io_error(ssize_t result, const char *operation, 
                   const char *function_name, int line_number);
void* handle_memory_error(void *ptr, const char *function_name, int line_number);
int handle_thread_error(int result, const char *function_name, int line_number);

int init_error_handling_system(void);
void cleanup_error_handling_system(void);

#endif
