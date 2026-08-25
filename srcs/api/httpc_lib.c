#include "../../includes/api/httpc.h"
#include "../../includes/core/config.h"
#include "../../includes/core/server.h"
#include "../../includes/core/server_internal.h"
#include "../../includes/core/logger.h"
#include <stdio.h>

int httpc_init(void) {
    if (init_logger_system() < 0) {
        return -1;
    }
    
    httpc_init_default_config();
    if (httpc_setup_signals() != 0) { 
        cleanup_logger_system();
        return -1; 
    }
    
    return (0);
}

void httpc_cleanup(void) {
    httpc_stop();
    httpc_destroy_default_server();
    cleanup_logger_system();
}
