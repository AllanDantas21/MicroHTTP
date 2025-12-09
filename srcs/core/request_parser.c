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

static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static char* url_decode(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char* decoded = malloc(len + 1);
    if (!decoded) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '%' && i + 2 < len) {
            int high = hex_to_int(str[i + 1]);
            int low = hex_to_int(str[i + 2]);
            if (high >= 0 && low >= 0) {
                decoded[j++] = (char)((high << 4) | low);
                i += 2;
                continue;
            }
        }
        if (str[i] == '+') {
            decoded[j++] = ' ';
            continue;
        }
        decoded[j++] = str[i];
    }
    decoded[j] = '\0';
    
    return decoded;
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

static void add_query_param(httpc_request_t* req, const char* name, const char* value) {
    if (!req || !name) return;
    
    QueryParam* param = malloc(sizeof(QueryParam));
    if (handle_memory_error(param, __func__, __LINE__) == NULL) {
        return;
    }
    
    param->name = string_dup(name);
    param->value = value ? string_dup(value) : string_dup("");
    param->next = req->parsed_query_params;
    req->parsed_query_params = param;
    
    if (!param->name || !param->value) {
        free(param->name);
        free(param->value);
        free(param);
    }
}

static void parse_query_string(httpc_request_t* req) {
    if (!req || !req->query_string) return;
    
    char* query_copy = string_dup(req->query_string);
    if (!query_copy) return;
    
    char* pair = query_copy;
    
    while (*pair != '\0') {
        char* next_pair = strchr(pair, '&');
        if (next_pair) {
            *next_pair = '\0';
        }
        
        char* equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            char* name = trim_whitespace(pair);
            char* value = trim_whitespace(equals + 1);
            
            if (name && strlen(name) > 0) {
                char* decoded_value = value ? url_decode(value) : NULL;
                add_query_param(req, name, decoded_value ? decoded_value : "");
                if (decoded_value) free(decoded_value);
            }
        } else {
            char* name = trim_whitespace(pair);
            if (name && strlen(name) > 0) {
                add_query_param(req, name, "");
            }
        }
        
        if (next_pair) {
            pair = next_pair + 1;
        } else {
            break;
        }
    }
    
    free(query_copy);
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
    
    if (req->query_string) {
        parse_query_string(req);
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
    
    QueryParam* qparam = req->parsed_query_params;
    while (qparam) {
        QueryParam* next = qparam->next;
        free(qparam->name);
        free(qparam->value);
        free(qparam);
        qparam = next;
    }
    
    PathParam* pparam = req->path_params;
    while (pparam) {
        PathParam* next = pparam->next;
        free(pparam->name);
        free(pparam->value);
        free(pparam);
        pparam = next;
    }
    
    req->parsed_headers = NULL;
    req->parsed_query_params = NULL;
    req->path_params = NULL;
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

const char* httpc_get_header(httpc_request_t* req, const char* name) {
    return httpc_get_header_value(req, name);
}

const char* httpc_get_query_param(httpc_request_t* req, const char* name) {
    if (!req || !name) return NULL;
    
    QueryParam* param = req->parsed_query_params;
    while (param) {
        if (param->name && strcmp(param->name, name) == 0) {
            return param->value;
        }
        param = param->next;
    }
    
    return NULL;
}

const char* httpc_get_path_param(httpc_request_t* req, const char* name) {
    if (!req || !name) return NULL;
    
    PathParam* param = req->path_params;
    while (param) {
        if (param->name && strcmp(param->name, name) == 0) {
            return param->value;
        }
        param = param->next;
    }
    
    return NULL;
}

const char* httpc_get_body(httpc_request_t* req) {
    if (!req) return NULL;
    return req->body;
}

const char* httpc_get_method(httpc_request_t* req) {
    if (!req) return NULL;
    return req->method;
}

const char* httpc_get_path(httpc_request_t* req) {
    if (!req) return NULL;
    return req->path;
}

