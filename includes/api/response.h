/**
 * @file response.h
 * @brief Cabeçalho para funções de resposta HTTP
 * @version 0.0.1
 * @author Aldantas
 * 
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include "../httpc.h"

/**
 * @brief Cria uma nova resposta HTTP
 * @param status_code Código de status HTTP
 * @param content_type Tipo de conteúdo
 * @param body Corpo da resposta
 * @return Ponteiro para a resposta criada ou NULL em caso de erro
 */
httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body);

/**
 * @brief Libera a memória de uma resposta HTTP
 * @param response Resposta a ser liberada
 */
void httpc_free_response(httpc_response_t* response);

/**
 * @brief Converte uma resposta HTTP para string
 * @param response Resposta a ser convertida
 * @return String da resposta HTTP ou NULL em caso de erro
 */
char* httpc_response_to_string(const httpc_response_t* response);

/**
 * @brief Define um cabeçalho na resposta HTTP
 * @param response Resposta onde definir o cabeçalho
 * @param key Chave do cabeçalho
 * @param value Valor do cabeçalho
 */
void httpc_set_header(httpc_response_t* response, const char* key, const char* value);

#endif // RESPONSE_H
