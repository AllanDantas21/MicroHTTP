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
#include "structs/enums.h"
#include "handlers/methods.h"
#include "router.h"
#include "server/server.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int port;
    int backlog;
    int max_clients;
    char* host;
    void (*on_request)(const char* method, const char* path, const char* body);
    void (*on_error)(const char* error);
} httpc_config_t;

typedef struct {
    int status_code;
    char* content_type;
    char* body;
    char* headers;
} httpc_response_t;

typedef struct {
    char* method;
    char* path;
    char* body;
    char* headers;
    char* query_string;
} httpc_request_t;

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
 * @param method Método HTTP (GET, POST, PUT, DELETE, etc.)
 * @param path Caminho da rota
 * @param handler Função que processa a requisição
 * @return 0 em caso de sucesso, -1 em caso de erro
 */
int httpc_add_route(const char* method, const char* path, route_handler handler);

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
 * @brief Limpa recursos da biblioteca
 */
void httpc_cleanup(void);

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

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

#define HTTPC_GET(path, handler) httpc_add_route("GET", path, handler)
#define HTTPC_POST(path, handler) httpc_add_route("POST", path, handler)
#define HTTPC_PUT(path, handler) httpc_add_route("PUT", path, handler)
#define HTTPC_DELETE(path, handler) httpc_add_route("DELETE", path, handler)
#define HTTPC_PATCH(path, handler) httpc_add_route("PATCH", path, handler)

#define HTTPC_OK(response) httpc_create_response(200, "text/plain", response)
#define HTTPC_CREATED(response) httpc_create_response(201, "text/plain", response)
#define HTTPC_NOT_FOUND(response) httpc_create_response(404, "text/plain", response)
#define HTTPC_BAD_REQUEST(response) httpc_create_response(400, "text/plain", response)
#define HTTPC_INTERNAL_ERROR(response) httpc_create_response(500, "text/plain", response)

#ifdef __cplusplus
}
#endif

#endif // HTTPC_H
