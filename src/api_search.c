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

void print_duckduckgo_results(cJSON *json) {
    cJSON *topics = cJSON_GetObjectItem(json, "RelatedTopics");
    if (!topics || !cJSON_IsArray(topics)) {
        printf("No results found.\n");
        return;
    }

    int count = 0;
    for (cJSON *item = topics->child; item; item = item->next) {
        // Some items may be "topics" with a "Topics" array inside
        cJSON *subtopics = cJSON_GetObjectItem(item, "Topics");
        if (subtopics && cJSON_IsArray(subtopics)) {
            for (cJSON *subitem = subtopics->child; subitem; subitem = subitem->next) {
                cJSON *text = cJSON_GetObjectItem(subitem, "Text");
                cJSON *url = cJSON_GetObjectItem(subitem, "FirstURL");
                if (text && url) {
                    printf("[%d] %s\n    %s\n\n", ++count, text->valuestring, url->valuestring);
                }
            }
        } else {
            cJSON *text = cJSON_GetObjectItem(item, "Text");
            cJSON *url = cJSON_GetObjectItem(item, "FirstURL");
            if (text && url) {
                printf("[%d] %s\n    %s\n\n", ++count, text->valuestring, url->valuestring);
            }
        }
    }
    if (count == 0) {
        printf("No results found.\n");
    }
}

int main(int argc, char *argv[]) {
    char *query = argv[1];
    cJSON *result = NULL;

    int status = api_search_duckduckgo(query, &result);
    if(status == 0 && result != NULL) {
        print_duckduckgo_results(result);
        cJSON_Delete(result);
    } else {
        fprintf(stderr, "Search failed or returned no results\n");
    }

    return 0;
}