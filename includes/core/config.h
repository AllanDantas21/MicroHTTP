#ifndef CONFIG_H
#define CONFIG_H

#include "../httpc.h"

int httpc_configure(const httpc_config_t* config);
const httpc_config_t* httpc_get_config(void);
void httpc_init_default_config(void);

#endif
