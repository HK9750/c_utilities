#include "../include/http.h"

const char* http_method_str(http_method_t method) {
    switch (method) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_PUT: return "PUT";
        case HTTP_DELETE: return "DELETE";
        case HTTP_HEAD: return "HEAD";
        case HTTP_OPTIONS: return "OPTIONS";
        case HTTP_PATCH: return "PATCH";
        default: return "UNKNOWN";
    }
}

const char* http_status_str(http_status_code_t status) {
    switch (status) {
        case HTTP_200_OK: return "200 OK";
        case HTTP_400_BAD_REQUEST: return "400 Bad Request";
        case HTTP_404_NOT_FOUND: return "404 Not Found";
        case HTTP_500_INTERNAL_SERVER_ERROR: return "500 Internal Server Error";
        default: return "UNKNOWN STATUS";
    }
}