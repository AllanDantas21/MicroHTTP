#include "../../includes/http.h"
#include "../../includes/router.h"
#include "../../includes/httpc.h"
#include "../../includes/structs/route.h"
#include <errno.h>

static RouteNode* routes_head = NULL;

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

int httpc_add_route(const char* method, const char* path, route_handler handler) {
    if (!method || !path || !handler) {
        errno = EINVAL;
        return -1;
    }
    
    router_add(method, path, handler);
    
    return 0;
}
