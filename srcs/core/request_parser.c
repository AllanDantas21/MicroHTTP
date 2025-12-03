#include "../../includes/core/request_parser.h"
#include "../../includes/core/utils.h"
#include "../../includes/core/error_handling.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static char* trim_whitespace(char* str) {
    if (!str) return NULL;
    
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

static void strtolower(char* str) {
    if (!str) return;
    for (char* p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
}

static void add_header(httpc_request_t* req, const char* name, const char* value) {
    if (!req || !name || !value) return;
    
    HttpHeader* header = malloc(sizeof(HttpHeader));
    if (handle_memory_error(header, __func__, __LINE__) == NULL) {
        return;
    }
    
    header->name = string_dup(name);
    header->value = string_dup(value);
    header->next = req->parsed_headers;
    req->parsed_headers = header;
    
    if (!header->name || !header->value) {
        free(header->name);
        free(header->value);
        free(header);
    }
}

static void parse_headers(httpc_request_t* req, const char* headers_start) {
    if (!req || !headers_start) return;
    
    char* headers_copy = string_dup(headers_start);
    if (!headers_copy) return;
    
    char* line = headers_copy;
    char* next_line;
    
    while ((next_line = strstr(line, "\r\n")) != NULL) {
        *next_line = '\0';
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* name = trim_whitespace(line);
            char* value = trim_whitespace(colon + 1);
            
            if (name && value && strlen(name) > 0 && strlen(value) > 0) {
                char* name_lower = string_dup(name);
                if (name_lower) {
                    strtolower(name_lower);
                    add_header(req, name_lower, value);
                    free(name_lower);
                }
            }
        }
        
        line = next_line + 2;
        if (*line == '\r' && *(line + 1) == '\n') {
            break;
        }
    }
    
    free(headers_copy);
}

int httpc_parse_request(const char* buffer, httpc_request_t* req) {
    if (!buffer || !req) {
        errno = EINVAL;
        return -1;
    }
    
    memset(req, 0, sizeof(httpc_request_t));
    
    const char* headers_end = strstr(buffer, "\r\n\r\n");
    if (!headers_end) {
        errno = EINVAL;
        return -1;
    }
    
    char method[16] = {0};
    char path[512] = {0};
    char http_version[16] = {0};
    
    if (sscanf(buffer, "%15s %511s %15s", method, path, http_version) != 3) {
        errno = EINVAL;
        return -1;
    }
    
    req->method = string_dup(method);
    if (!req->method) {
        return -1;
    }
    
    char* query_start = strchr(path, '?');
    if (query_start) {
        *query_start = '\0';
        req->path = string_dup(path);
        req->query_string = string_dup(query_start + 1);
    } else {
        req->path = string_dup(path);
        req->query_string = NULL;
    }
    
    if (!req->path) {
        free(req->method);
        return -1;
    }
    
    const char* headers_start = strstr(buffer, "\r\n");
    if (headers_start) {
        headers_start += 2;
        size_t headers_section_len = headers_end - headers_start;
        
        req->headers = malloc(headers_section_len + 1);
        if (handle_memory_error(req->headers, __func__, __LINE__) == NULL) {
            httpc_free_request(req);
            return -1;
        }
        
        memcpy(req->headers, headers_start, headers_section_len);
        req->headers[headers_section_len] = '\0';
        
        parse_headers(req, req->headers);
    }
    
    const char* body_start = headers_end + 4;
    size_t body_len = strlen(body_start);
    
    if (body_len > 0) {
        req->body = malloc(body_len + 1);
        if (handle_memory_error(req->body, __func__, __LINE__) == NULL) {
            httpc_free_request(req);
            return -1;
        }
        memcpy(req->body, body_start, body_len);
        req->body[body_len] = '\0';
    } else {
        req->body = NULL;
    }
    
    return 0;
}

void httpc_free_request(httpc_request_t* req) {
    if (!req) return;
    
    free(req->method);
    free(req->path);
    free(req->body);
    free(req->headers);
    free(req->query_string);
    
    HttpHeader* header = req->parsed_headers;
    while (header) {
        HttpHeader* next = header->next;
        free(header->name);
        free(header->value);
        free(header);
        header = next;
    }
    
    req->parsed_headers = NULL;
    memset(req, 0, sizeof(httpc_request_t));
}

const char* httpc_get_header_value(httpc_request_t* req, const char* name) {
    if (!req || !name) return NULL;
    
    char* name_lower = string_dup(name);
    if (!name_lower) return NULL;
    
    strtolower(name_lower);
    
    HttpHeader* header = req->parsed_headers;
    while (header) {
        if (header->name && strcmp(header->name, name_lower) == 0) {
            free(name_lower);
            return header->value;
        }
        header = header->next;
    }
    
    free(name_lower);
    return NULL;
}

