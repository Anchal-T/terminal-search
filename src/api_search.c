#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/http_client.h"
#include "../headers/terminal_ui.h"
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

    *result = json;
    http_response_free(&response);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <search query>\n", argv[0]);
        return 1;
    }
    
    char *query = argv[1];
    cJSON *result = NULL;

    // Perform search
    int status = api_search_duckduckgo(query, &result);
    if(status != 0 || result == NULL) {
        fprintf(stderr, "Search failed or returned no results\n");
        return 1;
    }

    // Parse results to UI state
    SearchState *state = parse_duckduckgo_results(result);
    if (!state || state->count == 0) {
        printf("No results found for: %s\n", query);
        cJSON_Delete(result);
        return 1;
    }

    // Initialize UI
    init_ui();

    // Main UI loop
    while(1){
        display_search_results(state, query);  // Fixed function name
        if(handle_input(state)){
            break;
        }
    }

    // Cleanup
    cleanup_ui();
    free_search_state(state);
    cJSON_Delete(result);
    
    return 0;
}
