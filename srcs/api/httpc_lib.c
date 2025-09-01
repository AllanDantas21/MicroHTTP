#include "../../includes/httpc.h"
#include "../../includes/core/config.h"
#include "../../includes/router.h"
#include "../../includes/server/server.h"
#include "../../includes/logger.h"
#include <stdio.h>

Router g_router = {0};

int httpc_init(void) {
    if (init_logger_system() < 0) {
        return -1;
    }
    
    httpc_init_default_config();
    g_router.routes_head = NULL;

    if (httpc_setup_signals() != 0) { 
        cleanup_logger_system();
        return -1; 
    }
    
    return (0);
}

void httpc_cleanup(void) {
    httpc_stop();
    router_cleanup(&g_router);
    cleanup_logger_system();
}
