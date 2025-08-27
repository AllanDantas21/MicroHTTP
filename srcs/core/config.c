#include "../../includes/core/config.h"
#include "../../includes/httpc.h"
#include "../../includes/constants/constants.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_HOST "127.0.0.1"

static httpc_config_t g_config = {0};

static int validate_config(const httpc_config_t* config) {
    if (!config) return (-1);
    
    if (config->port <= 0 || config->port > 65535) return (-1);
    if (config->backlog <= 0) return (-1);
    if (config->max_clients <= 0) return (-1);
    if (!config->host) return (-1);
    
    return (0);
}

static void apply_default_config(httpc_config_t* config) {
    if (config->port <= 0 || config->port > 65535) {
        config->port = PORT;
    }
    
    if (config->backlog <= 0) {
        config->backlog = BACKLOG;
    }
    
    if (config->max_clients <= 0) {
        config->max_clients = MAX_CLIENTS;
    }
    
    if (!config->host) {
        config->host = DEFAULT_HOST;
    }
}

int httpc_configure(const httpc_config_t* config) {
    if (!config) {
        errno = EINVAL;
        return (-1);
    }
    
    if (validate_config(config) != 0) {
        errno = EINVAL;
        return (-1);
    }
    
    g_config = *config;
    apply_default_config(&g_config);
    
    return (0);
}

const httpc_config_t* httpc_get_config(void) {
    return (&g_config);
}

void httpc_init_default_config(void) {
    g_config.port = PORT;
    g_config.backlog = BACKLOG;
    g_config.max_clients = MAX_CLIENTS;
    g_config.host = DEFAULT_HOST;
    g_config.on_request = NULL;
    g_config.on_error = NULL;
}
