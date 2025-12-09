#include "../../includes/api/response.h"
#include "../../includes/core/utils.h"
#include "../../includes/core/error_handling.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static int safe_snprintf(char** ptr, size_t* remaining, char** buffer_to_free,
                         const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int written = vsnprintf(*ptr, *remaining, format, args);
    va_end(args);
    
    if (written < 0 || (size_t)written >= *remaining) {
        if (buffer_to_free && *buffer_to_free) {
            free(*buffer_to_free);
            *buffer_to_free = NULL;
        }
        return 0;
    }
    
    *ptr += written;
    *remaining -= (size_t)written;
    return 1;
}

static int safe_memcpy(char** ptr, size_t* remaining, char** buffer_to_free,
                       const void* src, size_t len) {
    if (len > *remaining) {
        if (buffer_to_free && *buffer_to_free) {
            free(*buffer_to_free);
            *buffer_to_free = NULL;
        }
        return 0;
    }
    
    memcpy(*ptr, src, len);
    *ptr += len;
    *remaining -= len;
    return 1;
}

static int safe_add_terminator(char** ptr, size_t* remaining, char** buffer_to_free,
                               const char* terminator, size_t term_len) {
    if (term_len > *remaining) {
        if (buffer_to_free && *buffer_to_free) {
            free(*buffer_to_free);
            *buffer_to_free = NULL;
        }
        return 0;
    }
    
    memcpy(*ptr, terminator, term_len);
    *ptr += term_len;
    *remaining -= term_len;
    (*ptr)[0] = '\0';
    return 1;
}

httpc_response_t* httpc_create_response(int status_code, const char* content_type, const char* body) {
    httpc_response_t* response = malloc(sizeof(httpc_response_t));
    if (handle_memory_error(response, __func__, __LINE__) == NULL) {
        return NULL;
    }
    
    response->status_code = status_code;
    response->headers = NULL;
    
    response->content_type = content_type ? string_dup(content_type) : string_dup("text/plain");
    if (!response->content_type) {
        httpc_free_response(response);
        return NULL;
    }
    
    response->body = body ? string_dup(body) : string_dup("");
    if (!response->body) {
        httpc_free_response(response);
        return NULL;
    }
    
    return response;
}

void httpc_free_response(httpc_response_t* response) {
    if (!response) return;
    free(response->content_type);
    free(response->body);
    free(response->headers);
    free(response);
}

char* httpc_build_headers(int status_code, const char* content_type, size_t content_length, const char* extra_headers) {
    const char* status_text = get_status_text(status_code);

    size_t total_size = 0;
    total_size += snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", status_code, status_text);
    total_size += snprintf(NULL, 0, "Content-Type: %s\r\n", content_type ? content_type : "text/plain");
    total_size += snprintf(NULL, 0, "Content-Length: %zu\r\n", content_length);

    if (extra_headers && *extra_headers) {
        total_size += strlen(extra_headers);
        if (extra_headers[strlen(extra_headers) - 1] != '\n') {
            total_size += 2;
        }
    }

    total_size += 2;

    char* headers = malloc(total_size + 1);
    if (handle_memory_error(headers, __func__, __LINE__) == NULL) {
        return NULL;
    }

    char* ptr = headers;
    size_t remaining = total_size;
    
    if (!safe_snprintf(&ptr, &remaining, &headers, "HTTP/1.1 %d %s\r\n", status_code, status_text)) {
        return NULL;
    }
    
    if (!safe_snprintf(&ptr, &remaining, &headers, "Content-Type: %s\r\n", 
                       content_type ? content_type : "text/plain")) {
        return NULL;
    }
    
    if (!safe_snprintf(&ptr, &remaining, &headers, "Content-Length: %zu\r\n", content_length)) {
        return NULL;
    }

    if (extra_headers && *extra_headers) {
        size_t extra_len = strlen(extra_headers);
        if (!safe_memcpy(&ptr, &remaining, &headers, extra_headers, extra_len)) {
            return NULL;
        }
        
        if (extra_headers[strlen(extra_headers) - 1] != '\n') {
            if (!safe_memcpy(&ptr, &remaining, &headers, "\r\n", 2)) {
                return NULL;
            }
        }
    }

    if (!safe_add_terminator(&ptr, &remaining, &headers, "\r\n", 2)) {
        return NULL;
    }

    return headers;
}

char* build_response(int status_code, const char* content_type, const char* body) {
    const char* safe_body = body ? body : "";
    size_t body_len = strlen(safe_body);
    char* headers = httpc_build_headers(status_code, content_type, body_len, NULL);
    if (!headers) {
        return NULL;
    }

    size_t total = strlen(headers) + body_len;
    char* response = malloc(total + 1);
    if (handle_memory_error(response, __func__, __LINE__) == NULL) {
        free(headers);
        return NULL;
    }

    memcpy(response, headers, strlen(headers));
    memcpy(response + strlen(headers), safe_body, body_len);
    response[total] = '\0';

    free(headers);
    return response;
}

char* httpc_response_to_string(const httpc_response_t* response) {
    if (!response) {
        return NULL;
    }
    
    const char* status_text = get_status_text(response->status_code);
    size_t total_size = 0;
    
    total_size += snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", response->status_code, status_text);
    total_size += snprintf(NULL, 0, "Content-Type: %s\r\n", response->content_type);
    total_size += snprintf(NULL, 0, "Content-Length: %zu\r\n", strlen(response->body));
    
    if (response->headers) {
        total_size += strlen(response->headers);
        if (response->headers[strlen(response->headers) - 1] != '\n') {
            total_size += 2;
        }
    }
    
    total_size += 4;
    total_size += strlen(response->body);
    
    char* result = malloc(total_size + 1);
    if (handle_memory_error(result, __func__, __LINE__) == NULL) {
        return NULL;
    }
    
    char* ptr = result;
    size_t remaining = total_size;
    size_t body_len = strlen(response->body);
    
    if (!safe_snprintf(&ptr, &remaining, &result, "HTTP/1.1 %d %s\r\n", response->status_code, status_text)) {
        return NULL;
    }
    
    if (!safe_snprintf(&ptr, &remaining, &result, "Content-Type: %s\r\n", response->content_type)) {
        return NULL;
    }
    
    if (!safe_snprintf(&ptr, &remaining, &result, "Content-Length: %zu\r\n", body_len)) {
        return NULL;
    }
    
    if (response->headers) {
        size_t headers_len = strlen(response->headers);
        if (!safe_memcpy(&ptr, &remaining, &result, response->headers, headers_len)) {
            return NULL;
        }
        
        if (response->headers[strlen(response->headers) - 1] != '\n') {
            if (!safe_memcpy(&ptr, &remaining, &result, "\r\n", 2)) {
                return NULL;
            }
        }
    }
    
    if (!safe_memcpy(&ptr, &remaining, &result, "\r\n", 2)) {
        return NULL;
    }
    
    if (!safe_memcpy(&ptr, &remaining, &result, response->body, body_len)) {
        return NULL;
    }
    ptr[0] = '\0';
    
    return (result);
}

void httpc_set_header(httpc_response_t* response, const char* key, const char* value) {
    if (!response || !key || !value) {
        return;
    }
    
    size_t header_size = strlen(key) + strlen(value) + 4;
    
    if (response->headers) {
        size_t current_size = strlen(response->headers);
        size_t new_size = current_size + header_size;
        
        char* new_headers = realloc(response->headers, new_size + 1);
        if (handle_memory_error(new_headers, __func__, __LINE__) == NULL) {
            return;
        }
        
        response->headers = new_headers;
        int written = snprintf(response->headers + current_size, header_size + 1, "%s: %s\r\n", key, value);
        if (written < 0 || (size_t)written > header_size) {
            free(response->headers);
            response->headers = NULL;
            return;
        }
    } else {
        response->headers = malloc(header_size + 1);
        if (handle_memory_error(response->headers, __func__, __LINE__) == NULL) {
            return;
        }
        
        int written = snprintf(response->headers, header_size + 1, "%s: %s\r\n", key, value);
        if (written < 0 || (size_t)written > header_size) {
            free(response->headers);
            response->headers = NULL;
            return;
        }
    }
}
