#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "http_client.h"

typedef struct{
    char *data;
    size_t size;
} HTTPResponse;

size_t write_callback(void *response, size_t size, size_t nmemb, void *userp){
    size_t total_size = size * nmemb;
    HTTPResponse *res = (HTTPResponse *)userp;

    char *ptr = realloc(res->data, res->size + total_size + 1);
    if(ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory for response data\n");
        return 0; // Return 0 to indicate failure
    }

    res->data = ptr;
    memcpy(res->data + res->size, response, total_size);
    res->size += total_size;
    res->data[res->size] = '\0'; // Null-terminate the string

    return total_size;    
}

int http_search(const char *url, const char *query, HTTPResponse *response){
    if(url == NULL || query == NULL || response == NULL) {
        fprintf(stderr, "Invalid arguments provided to http_search\n");
        return -1; // Return -1 to indicate error
    }

    CURL *curl = curl_easy_init();
    if(curl == NULL) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return -1; // Return -1 to indicate error
    }

    response->data = malloc(1);
    response->data[0] = '\0';

    response->size = 0;
    
}
