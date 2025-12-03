#ifndef ROUTE_H
#define ROUTE_H

#include "httpc.h"

typedef httpc_response_t* (*route_handler)(httpc_request_t* req);

typedef struct RouteNode {
	char method[8];
	char route[256];
	route_handler handler;
	struct RouteNode* next;
} RouteNode;

typedef struct Router {
	RouteNode* routes_head;
} Router;

#endif
