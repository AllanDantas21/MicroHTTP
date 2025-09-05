#ifndef METHODS_H
#define METHODS_H

#include "api/http.h"

char* handle_get_request(const char* buffer);
char* handle_post_request(const char* buffer);
char* handle_unsupported_method(const char* method);

#endif
