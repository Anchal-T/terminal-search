#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/http_client.h"
#include <cjson/cJSON.h>


#define DUCKDUCKGO_API "https://api.duckduckgo.com/?q=%s&format=json&no_redirect=1&no_html=1&skip_disambig=1"

int api_search_duckduckgo(const char *query, cJSON **result) {
    if (query == NULL || result == NULL) {
        fprintf(stderr, "Invalid arguments provided to api_search_duckduckgo\n");
        return -1;
    }

    char url[1024];
    snprintf(url, sizeof(url), DUCKDUCKGO_API, query);
    HTTPResponse response = {NULL, 0};
    if(http_search(url, query, &response) != 0) {
        fprintf(stderr, "Failed to perform HTTP search\n");
        return -1;
    }

    cJSON *json = cJSON_Parse(response.data);
    if(!json){
        http_response_free(&response);
        return -1;
    }

    *result = json; // Pass parsed JSON back to caller

    http_response_free(&response);
    return 0;
}

int main(){
    const char *query = "fortnite";
    cJSON *result = NULL;

    int status = api_search_duckduckgo(query, &result);
    if(status == 0 && result != NULL) {
        char *json_string = cJSON_Print(result);
        printf("Search Results: %s\n", json_string);
        free(json_string);
        cJSON_Delete(result);
    } else {
        fprintf(stderr, "Search failed or returned no results\n");
    }

    return 0;
}