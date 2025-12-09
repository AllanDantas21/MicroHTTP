#include "../../includes/core/config.h"
#include "../../includes/api/httpc.h"
#include "../../includes/constants/constants.h"
#include "../../includes/core/logger.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static httpc_config_t g_config = {0};

static void default_on_request(const char* method, const char* path, const char* body) {
    log_info("Request received");
    printf("[INFO] %s %s\n", method, path);
    if (body && strlen(body) > 0) {
        printf("[BODY] %s\n", body);
    }
}

static void default_on_error(const char* error) {
    log_error(error);
    fprintf(stderr, "[ERROR] %s\n", error);
}

static int validate_config(const httpc_config_t* config) {
    if (!config) {
        return (-1);
    }
    
    if (config->port <= 0) {
        return (-1);
    }
    if (config->port > 65535) {
        return (-1);
    }
    
    if (config->backlog <= 0) {
        return (-1);
    }
    
    if (config->max_clients <= 0) {
        return (-1);
    }
    
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
}

int httpc_configure(const httpc_config_t* config) {
    if (!config) {
        errno = EINVAL;
        return (-1);
    }
    
    errno = 0;
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
    g_config.on_request = default_on_request;
    g_config.on_error = default_on_error;
}
