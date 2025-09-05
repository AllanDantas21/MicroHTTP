#ifndef ROUTES_H
#define ROUTES_H

#include "http.h"

char* route_get_root(const char* buffer);
char* route_get_health(const char* buffer);
char* route_post_echo(const char* buffer);

#endif
