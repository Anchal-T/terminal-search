#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/html_parser.h"
#include "../headers/http_client.h"

char *parse_webpage(const char *url) {
    HTTPResponse response = {NULL, 0};
    if (http_search(url, "", &response) != 0) {
        return NULL; // Failed to fetch webpage
    }
    char *parsed_content = html_parse(response.data);
    http_response_free(&response);
    return parsed_content; // Return parsed HTML content or NULL if parsing failed
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *content = parse_webpage(argv[1]);
    if (content) {
        printf("Parsed content:\n%s\n", content);
        free(content);
    } else {
        fprintf(stderr, "Failed to parse webpage.\n");
    }

    return 0;
}