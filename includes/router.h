#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"

typedef char* (*route_handler)(const char* buffer);

void router_init(void);
void router_add(const char* method, const char* route, route_handler handler);
route_handler router_match(const char* method, const char* route);
void router_cleanup(void);

int httpc_add_route(const char* method, const char* path, route_handler handler);

#endif
