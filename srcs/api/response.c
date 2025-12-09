#include "../../includes/api/response.h"
#include "../../includes/core/utils.h"
#include "../../includes/core/error_handling.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    
    int written = snprintf(ptr, remaining, "HTTP/1.1 %d %s\r\n", status_code, status_text);
    if (written < 0 || (size_t)written >= remaining) {
        free(headers);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    written = snprintf(ptr, remaining, "Content-Type: %s\r\n", content_type ? content_type : "text/plain");
    if (written < 0 || (size_t)written >= remaining) {
        free(headers);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    written = snprintf(ptr, remaining, "Content-Length: %zu\r\n", content_length);
    if (written < 0 || (size_t)written >= remaining) {
        free(headers);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;

    if (extra_headers && *extra_headers) {
        size_t extra_len = strlen(extra_headers);
        if (extra_len > remaining - 3) {
            free(headers);
            return NULL;
        }
        memcpy(ptr, extra_headers, extra_len);
        ptr += extra_len;
        remaining -= extra_len;
        
        if (extra_headers[strlen(extra_headers) - 1] != '\n') {
            if (remaining < 3) {
                free(headers);
                return NULL;
            }
            memcpy(ptr, "\r\n", 2);
            ptr += 2;
            remaining -= 2;
        }
    }

    if (remaining < 2) {
        free(headers);
        return NULL;
    }
    memcpy(ptr, "\r\n", 2);
    ptr[2] = '\0';

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
    
    int written = snprintf(ptr, remaining, "HTTP/1.1 %d %s\r\n", response->status_code, status_text);
    if (written < 0 || (size_t)written >= remaining) {
        free(result);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    written = snprintf(ptr, remaining, "Content-Type: %s\r\n", response->content_type);
    if (written < 0 || (size_t)written >= remaining) {
        free(result);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    size_t body_len = strlen(response->body);
    written = snprintf(ptr, remaining, "Content-Length: %zu\r\n", body_len);
    if (written < 0 || (size_t)written >= remaining) {
        free(result);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    if (response->headers) {
        size_t headers_len = strlen(response->headers);
        if (headers_len > remaining - 3) {
            free(result);
            return NULL;
        }
        memcpy(ptr, response->headers, headers_len);
        ptr += headers_len;
        remaining -= headers_len;
        
        if (response->headers[strlen(response->headers) - 1] != '\n') {
            if (remaining < 3) {
                free(result);
                return NULL;
            }
            memcpy(ptr, "\r\n", 2);
            ptr += 2;
            remaining -= 2;
        }
    }
    
    if (remaining < 3) {
        free(result);
        return NULL;
    }
    memcpy(ptr, "\r\n", 2);
    ptr += 2;
    remaining -= 2;
    
    if (body_len > remaining) {
        free(result);
        return NULL;
    }
    memcpy(ptr, response->body, body_len);
    ptr[body_len] = '\0';
    
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
