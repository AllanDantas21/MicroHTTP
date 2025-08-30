#ifndef ROUTE_H
#define ROUTE_H

typedef char* (*route_handler)(const char* buffer);

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
