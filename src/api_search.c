#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../headers/http_client.h"
#include "../headers/terminal_ui.h"
#include "../headers/env_loader.h"
#include <cjson/cJSON.h>

#define GOOGLE_API "https://www.googleapis.com/customsearch/v1?key=%s&cx=%s&q=%s&num=10"

// Function prototype
void url_encode(const char *src, char *dest, size_t dest_size);

int api_search_google(const char *query, cJSON **result) {
    if (query == NULL || result == NULL) {
        fprintf(stderr, "Invalid arguments provided to api_search_google\n");
        return -1;
    }

    // Get API credentials from environment
    char *api_key = get_env_value("GOOGLE_API_KEY");
    char *cx = get_env_value("GOOGLE_CX");
    
    if (!api_key || !cx) {
        fprintf(stderr, "Error: GOOGLE_API_KEY or GOOGLE_CX not found in .env file\n");
        return -1;
    }

    // URL encode the query
    char encoded_query[1024];
    url_encode(query, encoded_query, sizeof(encoded_query));

    char url[2048];
    snprintf(url, sizeof(url), GOOGLE_API, api_key, cx, encoded_query);
    
    HTTPResponse response = {NULL, 0};
    if(http_search(url, query, &response) != 0) {
        fprintf(stderr, "Failed to perform HTTP search\n");
        return -1;
    }

    // printf("Debug: Response size: %zu\n", response.size);
    if (response.size == 0) {
        fprintf(stderr, "Empty response from Google API\n");
        http_response_free(&response);
        return -1;
    }

    // Debug: Print the raw response to see what Google is returning
    // printf("Debug: Raw response:\n%s\n", response.data);

    cJSON *json = cJSON_Parse(response.data);
    if(!json){
        fprintf(stderr, "Failed to parse JSON response\n");
        printf("Response: %s\n", response.data);
        http_response_free(&response);
        return -1;
    }

    *result = json;
    http_response_free(&response);
    return 0;
}

// Simple URL encoding function
void url_encode(const char *src, char *dest, size_t dest_size) {
    const char *hex = "0123456789ABCDEF";
    size_t i = 0, j = 0;
    
    while (src[i] && j < dest_size - 4) {
        if (isalnum(src[i]) || src[i] == '-' || src[i] == '_' || src[i] == '.' || src[i] == '~') {
            dest[j++] = src[i];
        } else if (src[i] == ' ') {
            dest[j++] = '+';
        } else {
            dest[j++] = '%';
            dest[j++] = hex[(unsigned char)src[i] >> 4];
            dest[j++] = hex[(unsigned char)src[i] & 15];
        }
        i++;
    }
    dest[j] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <search query>\n", argv[0]);
        return 1;
    }
    
    // Load environment variables from .env file
    if (load_env_file(".env") != 0) {
        fprintf(stderr, "Warning: Could not load .env file\n");
    }
    
    char *query = argv[1];
    cJSON *result = NULL;

    // Perform search using Google API
    int status = api_search_google(query, &result);
    if(status != 0 || result == NULL) {
        fprintf(stderr, "Search failed or returned no results\n");
        return 1;
    }

    // Parse results to UI state
    SearchState *state = parse_google_results(result);
    if (!state || state->count == 0) {
        printf("No results found for: %s\n", query);
        cJSON_Delete(result);
        return 1;
    }

    // Initialize UI
    init_ui();

    // Main UI loop
    while(1){
        display_search_results(state, query);
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
