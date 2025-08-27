#ifndef ROUTE_H
#define ROUTE_H

#include "../router.h"

typedef struct RouteNode {
	char method[8];
	char route[256];
	route_handler handler;
	struct RouteNode* next;
} RouteNode;

#endif
