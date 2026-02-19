#ifndef RESPONSE_H
#define RESPONSE_H
   
#include "http.h"
#include<stdio.h>
typedef struct {
    http_status_code_t status_code;
    char* headers;
    char* body;
    size_t body_len;
} http_response_t;

void response_init(http_response_t* res);
void response_add_status(http_response_t* res, http_status_code_t status_code);
void response_add_header(http_response_t* res,const char* key,const char* value);
char* response_build(http_response_t* res,size_t* len);
void response_free(http_response_t* res);

#endif