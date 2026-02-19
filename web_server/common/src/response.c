#include "../include/response.h"
#include <stdlib.h>
#include <string.h>

void response_init(http_response_t* res) {
    res->status_code = HTTP_200_OK;
    res->headers = NULL;
    res->body_len = 0;
    res->body = NULL; 
}

void response_add_status(http_response_t* res, http_status_code_t status_code){
    res->status_code = status_code;
}

void response_add_header(http_response_t* res,const char* key,const char* value) {
    char header_line[256];
    sprintf(header_line,"%s: %s\r\n", key, value);
    if(!res->headers) {
        res->headers = strdup(header_line);
    } else {
        size_t new_len = strlen(res->headers) + strlen(header_line) + 1;
        char* new_headers = realloc(res->headers, new_len);
        if(new_headers) {
            strcat(new_headers, header_line);
            res->headers = new_headers;
        }
    }
}

char* response_build(http_response_t* res,size_t* len) {
    char status_line[64];
    sprintf(status_line, "HTTP/1.1 %d %s\r\n", res->status_code, http_status_str(res->status_code));
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

void response_free(http_response_t* res) {
    free(res->headers);
    free(res->body);
}