#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJson/cJSON.h>

#define DUCKDUCKGO_API = "https://api.duckduckgo.com/?q=%s&format=json&no_redirect=1&no_html=1&skip_disambig=1"

int api_search_duckduckgo(const char *query, cJSON **result) {
    if (query == NULL || result == NULL) {
        fprintf(stderr, "Invalid arguments provided to api_search_duckduckgo\n");
        return -1; // Return -1 to indicate error
    }

    char url[1024];
    snsprintf(url, sizeof(url), DUCKDUCKGO_API, query);
    HTTPResponse response = {null, 0};
    if(http_search(url, query, &response) != 0) {
        fprintf(stderr, "Failed to perform HTTP search\n");
        return -1; // Return -1 to indicate error
    }

    cJSON *json = cJSON_Parse(response.data);
    if(!json){
        http_response_free(&response);
        return -1;
    }

    cJSON *topics = cJSON_GetObjectItem(json, "RelatedTopics");
    if(!cJSON_IsArray(topics)) {
        cJSON_Delete(json);
        http_response_free(&response);
        return -1; // Return -1 to indicate error
    }

    int count = cJSON_GetArraySize(topics);
    response->result = calloc(count, sizeof(topics));
    response->count = 0;

    for(int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(topics, i);
        cJSON *text = cJSON_GetObjectItem(item, "Text");
        cJSON *first_url = cJSON_GetObjectItem(item, "FirstURL");

        if(text && first_url){
            response->results[response->count].title = strdup(text->valuestring);
            response->results[response->count].url = strdup(first_url->valuestring);
            response->results[response->count].snippet = NULL; // DuckDuckGo doesn't provide snippet
            response->count++;

        }
    }

    cJSON_Delete(json);
    http_response_free(&response);
    return 0;
}