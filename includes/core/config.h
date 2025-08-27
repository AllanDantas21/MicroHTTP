/**
 * @file config.h
 * @brief Cabeçalho para funções de configuração do servidor
 * @version 0.0.1
 * @author Aldantas
 * 
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../httpc.h"

/**
 * @brief Configura o servidor com os parâmetros fornecidos
 * @param config Configuração do servidor
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_configure(const httpc_config_t* config);

/**
 * @brief Obtém a configuração atual do servidor
 * @return Ponteiro para a configuração atual
 */
const httpc_config_t* httpc_get_config(void);

/**
 * @brief Inicializa a configuração com valores padrão
 */
void httpc_init_default_config(void);

#endif // CONFIG_H
