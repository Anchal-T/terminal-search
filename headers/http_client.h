#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

typedef struct {
    char *data;
    size_t size;
} HTTPResponse;

size_t write_callback(void *response, size_t size, size_t nmemb, void *userp);
int http_search(const char *url, const char *query, HTTPResponse *response);
void http_response_free(HTTPResponse *response);

#endif /* HTTP_CLIENT_H */