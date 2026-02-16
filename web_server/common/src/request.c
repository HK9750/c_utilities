// "POST /login HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/json\r\nContent-Length: 18\r\n\r\n{\"user\":\"admin\"}"
#include "../include/request.h"
#include<string.h>
#include<stdlib.h>

void http_request_init(http_request_t* req) {
    req->method = HTTP_GET;
    req->path = NULL;
    req->version = NULL;
    req->body = NULL;
    req->headers = NULL;
    req->len = 0;
}

static char* get_token(char** src) {
    char* start = src;
    while(**src && **src != ' ' && **src !='\r' && **src !='\n') (*src)++;
    if(*src == start) return NULL;
    size_t len = src - start;
    char* token = malloc(len + 1);
    memcpy(token,start,len);
    token[len] = '\0';
    return token;
}

void assign_http_method(http_request_t** req,char * method_str) {
    if(strcmp(method_str,"GET")) (*req)->method = HTTP_GET;
    else if(strcmp(method_str,"POST")) (*req)->method = HTTP_POST;
    else if(strcmp(method_str,"PUT")) (*req)->method = HTTP_PUT;
    else if(strcmp(method_str,"DELETE")) (*req)->method = HTTP_DELETE;
    else if(strcmp(method_str,"PATCH")) (*req)->method = HTTP_PATCH;
    else if(strcmp(method_str,"HEAD")) (*req)->method = HTTP_HEAD;
    else (*req)->method = HTTP_OPTIONS;
}

void http_request_parse(http_request_t* req, const char* raw_request){
    char *buf = strdup(raw_request);
    char *ptr = buf;
    char* method_str = get_token(&ptr);
    if(method_str) {
        assign_http_method(&req,method_str);
        free(method_str);
    }
    if(*ptr ==  ' ') ptr++;
    char* path_str = get_token(&ptr);
    if(path_str) {
        req->path = path_str;
    }
    if(*ptr == ' ') ptr++;
    req->version  = get_token(&ptr);
    char* headers_start = strstr(buf,"\r\n");
    if(headers_start){
        headers_start+=2;
        char* body_start = strstr(headers_start,"\r\n\r\n");
        if(body_start){
            size_t headers_len = body_start - headers_start;
            req->headers = malloc(headers_len + 1);
            memcpy(req->headers, headers_start, headers_len);
            req->headers[headers_len] = '\0';
            body_start += 4;
            req->len = strlen(body_start);
            req->body = malloc(req->len + 1);
            memcpy(req->body, body_start, req->len + 1);
        }   
        else {
            req->headers = strdup(headers_start);
        }
    }
    free(buf);
}

void http_request_free(http_request_t* req) {
    free(req->method);
    free(req->method);
    free(req->path);
    free(req->headers);
    free(req->body);
    free(req->len);
}