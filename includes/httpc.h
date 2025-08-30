/**
 * @file httpc.h
 * @brief HTTP.c - Micro Framework HTTP
 * @version 1.0.0
 * @author Allan Dantas
 * 
 */

#ifndef HTTPC_H
#define HTTPC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "constants/constants.h"
#include "constants/macros.h"
#include "structs/enums.h"
#include "structs/httpc.h"
#include "handlers/methods.h"
#include "router.h"
#include "server/server.h"

#ifdef __cplusplus
extern "C" {
#endif

// Instância global do router
extern Router g_router;

/**
 * @brief Inicializa a biblioteca HTTP.c
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_init(void);

/**
 * @brief Configura o servidor com as opções especificadas
 * @param config Configuração do servidor
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_configure(const httpc_config_t* config);

/**
 * @brief Adiciona uma rota ao servidor
 * @param router Ponteiro para a instância do router
 * @param method Método HTTP (GET, POST, PUT, DELETE, etc.)
 * @param path Caminho da rota
 * @param handler Função que processa a requisição
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_add_route(Router* router, const char* method, const char* path, route_handler handler);

/**
 * @brief Inicia o servidor HTTP
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_start(void);

/**
 * @brief Para o servidor HTTP
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_stop(void);

/**
 * @brief Executa o servidor em loop até receber sinal de parada
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_run(void);

/**
 * @brief Limpa recursos da biblioteca
 */
void httpc_cleanup(void);

/**
 * @brief Cria uma resposta HTTP
 * @param status_code Código de status HTTP
 * @param content_type Tipo de conteúdo
 * @param body Corpo da resposta
 * @return Estrutura de resposta HTTP
 */
httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body);

/**
 * @brief Libera uma resposta HTTP
 * @param response Resposta a ser liberada
 */
void httpc_free_response(httpc_response_t* response);

/**
 * @brief Converte uma resposta HTTP para string
 * @param response Resposta HTTP
 * @return String formatada da resposta
 */
char* httpc_response_to_string(const httpc_response_t* response);

/**
 * @brief Define um header personalizado na resposta
 * @param response Resposta HTTP
 * @param key Chave do header
 * @param value Valor do header
 */
void httpc_set_header(httpc_response_t* response, const char* key, const char* value);

/**
 * @brief Verifica se o servidor está rodando
 * @return 1 se estiver rodando, 0 caso contrário
 */
int httpc_is_running(void);

/**
 * @brief Obtém a configuração atual do servidor
 * @return Ponteiro para a configuração atual
 */
const httpc_config_t* httpc_get_config(void);

/**
 * @brief Obtém o socket do servidor
 * @return Descritor do socket ou -1 se não estiver ativo
 */
int httpc_get_server_socket(void);



#ifdef __cplusplus
}
#endif

#endif
