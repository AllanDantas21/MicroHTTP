#include "../../../includes/api/http.h"
#include "../../../includes/core/router.h"
#include "../../../includes/api/httpc.h"
#include "../../../includes/structs/route.h"
#include <stdbool.h>
#include <errno.h>

static char* normalize_route(const char* route) {
	if (!route) return NULL;
	
	while (*route == '/') { route++; }
	if (*route == '\0') { return ""; }
	return (char*)route;
}

static bool copy_node(Router* router, const char* method, const char* route, route_handler handler) {
	RouteNode* node = malloc(sizeof(RouteNode));
	if (handle_memory_error(node, __func__, __LINE__) == NULL) { 
		return false; 
	}

	strncpy(node->method, method, sizeof(node->method) - 1);
	node->method[sizeof(node->method) - 1] = '\0';
	strncpy(node->route, route, sizeof(node->route) - 1);
	node->route[sizeof(node->route) - 1] = '\0';

	node->handler = handler;
	node->next = router->routes_head;
	router->routes_head = node;
	return true;
}

void router_add(Router* router, const char* method, const char* route, route_handler handler) {
	if (!router || !method || !route || !handler) return;

	route = normalize_route(route);
	if(!copy_node(router, method, route, handler)) { return; }
}

route_handler router_match(Router* router, const char* method, const char* route) {
	if (!router) return NULL;
	
	for (RouteNode* it = router->routes_head; it; it = it->next) {
		if (strcmp(it->method, method) == 0 && strcmp(it->route, route) == 0) {
			return it->handler;
		}
	}
	return (NULL);
}

void router_cleanup(Router* router) {
	if (!router) return;
	
	RouteNode* curr = router->routes_head;
	RouteNode* next;
	
	while (curr != NULL) {
		next = curr->next;
		free(curr);
		curr = next;
	}
	
	router->routes_head = NULL;
}

int httpc_add_route(Router* router, const char* method, const char* path, route_handler handler) {
    if (!router || !method || !path || !handler) {
        errno = EINVAL;
        return (-1);
    }
    
    router_add(router, method, path, handler);
    return (0);
}
