#ifndef REQUEST_H
#define REQUEST_H

#include "http.h"
#include "stddef.h"

typedef struct {
    http_method_t method;
    char* path;
    char* version;
    char* body;
    char* headers;
    size_t len;
} http_request_t;

void assign_http_method(http_request_t** req,char * method_str);
void http_request_init(http_request_t* req);
void http_request_parse(http_request_t* req, const char* raw_request);
void http_request_free(http_request_t* req);

#endif