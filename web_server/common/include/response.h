#ifndef RESPONSE_H
#define RESPONSE_H

#include "http.h"
#include <stdio.h>
typedef struct {
    http_status_code_t status_code;
    char* headers;
    char* body;
    size_t body_len;
} http_response_t;

void http_response_init(http_response_t* res);
void http_response_set_status(http_response_t* res, http_status_code_t status_code);
void http_response_add_header(http_response_t* res,const char* key,const char* value);
void http_response_set_body(http_response_t* res,const char* body,size_t len);
char* http_response_build(http_response_t* res,size_t* len);
void http_response_free(http_response_t* res);
#endif