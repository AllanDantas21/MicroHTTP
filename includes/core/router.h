#ifndef ROUTER_H
#define ROUTER_H

#include "structs/route.h"

typedef char* (*route_handler)(const char* buffer);

void router_add(Router* router, const char* method, const char* route, route_handler handler);
route_handler router_match(Router* router, const char* method, const char* route);
void router_cleanup(Router* router);
int httpc_add_route(Router* router, const char* method, const char* path, route_handler handler);
char* route_get_root(const char* buffer);
char* route_get_health(const char* buffer);
char* route_post_echo(const char* buffer);

#endif
