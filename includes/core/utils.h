/**
 * @file utils.h
 * @brief Cabeçalho para funções utilitárias
 * @version 0.0.1
 * @author Aldantas
 * 
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Duplica uma string
 * @param str String a ser duplicada
 * @return Ponteiro para a nova string ou NULL em caso de erro
 */
char* string_dup(const char* str);

/**
 * @brief Obtém o texto correspondente ao código de status HTTP
 * @param status_code Código de status HTTP
 * @return Texto do status HTTP
 */
const char* get_status_text(int status_code);

#endif // UTILS_H
