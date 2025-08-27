#include "../../includes/httpc.h"
#include "../../includes/core/config.h"
#include "../../includes/router.h"
#include "../../includes/server/server.h"

int httpc_init(void) {
    httpc_init_default_config();
    router_init();

    if (httpc_setup_signals() != 0) { return -1; }
    return (0);
}

void httpc_cleanup(void) {
    httpc_stop();
}
