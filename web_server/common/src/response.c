#include "../include/response.h"
#include <stdlib.h>
#include <string.h>

void http_response_init(http_response_t* res) {
    res->status_code = HTTP_200_OK;
    res->headers = NULL;
    res->body_len = 0;
    res->body = NULL; 
}

void http_response_set_status(http_response_t* res, http_status_code_t status_code) {
    res->status_code = status_code;
}

void http_response_add_header(http_response_t* res, const char* key, const char* value) {
    size_t needed = strlen(key) + strlen(value) + 5; 
    char* header_line = malloc(needed);
    if (!header_line) return;
    snprintf(header_line, needed, "%s: %s\r\n", key, value);

    if (!res->headers) {
        res->headers = strdup(header_line);
    } else {
        size_t new_len = strlen(res->headers) + strlen(header_line) + 1;
        char* new_headers = realloc(res->headers, new_len);
        if (new_headers) {
            strcat(new_headers, header_line);
            res->headers = new_headers;
        }
    }
    free(header_line);
}

void http_response_set_body(http_response_t* res, const char* body, size_t len) {
    free(res->body);
    res->body = malloc(len + 1);
    memcpy(res->body, body, len);
    res->body[len] = '\0';
    res->body_len = len;
}


char* http_response_build(http_response_t* res, size_t* len) {
    char status_line[128];
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",res->status_code, http_status_str(res->status_code));
    size_t total_len = strlen(status_line);
    if (res->headers) total_len += strlen(res->headers);
    total_len += 2;
    if (res->body) total_len += res->body_len;
    char* response = malloc(total_len + 1);
    char* ptr = response;
    ptr += sprintf(ptr, "%s", status_line);
    if (res->headers) ptr += sprintf(ptr, "%s", res->headers);
    ptr += sprintf(ptr, "\r\n");
    if (res->body) {
        memcpy(ptr, res->body, res->body_len);
        ptr += res->body_len;
    }
    *ptr = '\0';
    *len = ptr - response;
    return response;
}

void http_response_free(http_response_t* res) {
    free(res->headers);
    free(res->body);
}