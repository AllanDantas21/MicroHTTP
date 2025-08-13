#include "http.h"
#include "router.h"
#include "routes.h"

typedef struct RouteNode {
	char method[8];
	char route[256];
	route_handler handler;
	struct RouteNode* next;
} RouteNode;

static RouteNode* routes_head = NULL;

void build_routes(void) {
	router_add("GET", "", route_get_root);
	router_add("GET", "health", route_get_health);
	router_add("POST", "echo", route_post_echo);
}

void router_init(void) {
	routes_head = NULL;
}

void router_add(const char* method, const char* route, route_handler handler) {
	if (!method || !route || !handler) return;

	RouteNode* node = (RouteNode*)malloc(sizeof(RouteNode));
	if (!node) {
		return;
	}

	strncpy(node->method, method, sizeof(node->method) - 1);
	node->method[sizeof(node->method) - 1] = '\0';

	strncpy(node->route, route, sizeof(node->route) - 1);
	node->route[sizeof(node->route) - 1] = '\0';

	node->handler = handler;
	node->next = routes_head;
	routes_head = node;
}

route_handler router_match(const char* method, const char* route) {
	for (RouteNode* it = routes_head; it; it = it->next) {
		if (strcmp(it->method, method) == 0 && strcmp(it->route, route) == 0) {
			return it->handler;
		}
	}
	return NULL;
}
