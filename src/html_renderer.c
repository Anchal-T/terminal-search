#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include "../headers/html_renderer.h"

#define RESET "\033[0m"
#define BOLD "\033[1m"
#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"

#define INITIAL_BUFFER_CAPACITY 1024

// Hash values for HTML elements
#define HASH_TITLE 1
#define HASH_H1 2
#define HASH_H2 3
#define HASH_H3 4
#define HASH_P 5
#define HASH_A 6
#define HASH_LI 7
#define HASH_BR 8
#define HASH_STYLE 9
#define HASH_SCRIPT 10
#define HASH_HEAD 11
#define HASH_META 12
#define HASH_LINK 13
#define HASH_NOSCRIPT 14

static int get_element_hash(const char *name) {
    if(strcasecmp(name, "title") == 0) return HASH_TITLE;
    if(strcasecmp(name, "h1") == 0) return HASH_H1;
    if(strcasecmp(name, "h2") == 0) return HASH_H2;
    if(strcasecmp(name, "h3") == 0) return HASH_H3;
    if(strcasecmp(name, "p") == 0) return HASH_P;
    if(strcasecmp(name, "a") == 0) return HASH_A;
    if(strcasecmp(name, "li") == 0) return HASH_LI;
    if(strcasecmp(name, "br") == 0) return HASH_BR;
    if(strcasecmp(name, "style") == 0) return HASH_STYLE;
    if(strcasecmp(name, "script") == 0) return HASH_SCRIPT;
    if(strcasecmp(name, "head") == 0) return HASH_HEAD;
    if(strcasecmp(name, "meta") == 0) return HASH_META;
    if(strcasecmp(name, "link") == 0) return HASH_LINK;
    if(strcasecmp(name, "noscript") == 0) return HASH_NOSCRIPT;
    return 0; // Unknown element
}

static void render_node(xmlNode *node, char **output, size_t *size, size_t *capacity){
    for(xmlNode *cur = node; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            const char *name = (const char *)cur->name;
            int element_hash = get_element_hash(name);
            
            // Skip non-content elements
            switch(element_hash) {
                case HASH_STYLE:
                case HASH_SCRIPT:
                case HASH_HEAD:
                case HASH_META:
                case HASH_LINK:
                case HASH_NOSCRIPT:
                    continue;
            }

            // Opening tags
            switch(element_hash) {
                case HASH_TITLE:
                    append_to_output(output, size, capacity, "\n" BOLD CYAN "--- ");
                    break;
                case HASH_H1:
                    append_to_output(output, size, capacity, "\n\n" BOLD YELLOW);
                    break;
                case HASH_H2:
                    append_to_output(output, size, capacity, "\n\n" BOLD GREEN);
                    break;
                case HASH_H3:
                    append_to_output(output, size, capacity, "\n\n" BOLD);
                    break;
                case HASH_P:
                    append_to_output(output, size, capacity, "\n");
                    break;
                case HASH_A:
                    append_to_output(output, size, capacity, BLUE "[");
                    break;
                case HASH_LI:
                    append_to_output(output, size, capacity, "\n  • ");
                    break;
                case HASH_BR:
                    append_to_output(output, size, capacity, "\n");
                    continue; // br is self-closing
            }
        }
        else if(cur->type == XML_TEXT_NODE) {
            const char *content = (const char *)cur->content;
            if(content) {
                char *cleaned = clean_text(content);
                if(cleaned && strlen(cleaned) > 0) {
                    append_to_output(output, size, capacity, cleaned);
                }
                free(cleaned);
            }
        }

        // Recursively render children
        render_node(cur->children, output, size, capacity);
        
        // Closing tags
        if (cur->type == XML_ELEMENT_NODE) {
            const char *name = (const char *)cur->name;
            int element_hash = get_element_hash(name);
            
            switch(element_hash) {
                case HASH_TITLE:
                    append_to_output(output, size, capacity, " ---" RESET "\n");
                    break;
                case HASH_H1:
                case HASH_H2:
                case HASH_H3:
                    append_to_output(output, size, capacity, RESET "\n");
                    break;
                case HASH_P:
                    append_to_output(output, size, capacity, "\n");
                    break;
                case HASH_A:
                    append_to_output(output, size, capacity, "]" RESET);
                    break;
            }
        }
    }
}

static void append_to_output(char **output, size_t *size, size_t *capacity, const char *text) {
    size_t text_len = strlen(text);
    while(*size + text_len + 1 > *capacity) {
        *capacity *= 2;
        char *new_output = realloc(*output, *capacity);
        if(new_output == NULL) {
            fprintf(stderr, "Failed to allocate memory for output\n");
            return;
        }
        *output = new_output;
    }
    memcpy(*output + *size, text, text_len);
    *size += text_len;
    (*output)[*size] = '\0';
}

static char *clean_text(const char *text) {
    if(text == NULL) return NULL;

    size_t len = strlen(text);
    char *cleaned = malloc(len + 1);
    if(cleaned == NULL) {
        fprintf(stderr, "Failed to allocate memory for cleaned text\n");
        return NULL;
    }

    size_t j = 0;
    int in_whitespace = 0;
    
    // Clean and normalize whitespace
    for(size_t i = 0; i < len; i++) {
        if(text[i] == ' ' || text[i] == '\t' || text[i] == '\n' || text[i] == '\r') {
            if(!in_whitespace && j > 0) {
                cleaned[j++] = ' ';
                in_whitespace = 1;
            }
        } else {
            cleaned[j++] = text[i];
            in_whitespace = 0;
        }
    }
    
    // Remove trailing whitespace
    while(j > 0 && cleaned[j-1] == ' ') {
        j--;
    }
    
    cleaned[j] = '\0';
    return cleaned;
}

char *html_renderer(const char *html_content) {
    if(html_content == NULL) return NULL;

    htmlDocPtr doc = htmlReadMemory(html_content, strlen(html_content), NULL, NULL, 
                                    HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if(doc == NULL) {
        fprintf(stderr, "Failed to parse HTML content\n");
        return NULL;
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    if(root == NULL) {
        xmlFreeDoc(doc);
        fprintf(stderr, "No root element found in HTML document\n");
        return NULL;
    }

    char *output = malloc(INITIAL_BUFFER_CAPACITY);
    if(output == NULL) {
        xmlFreeDoc(doc);
        fprintf(stderr, "Failed to allocate memory for output\n");
        return NULL;
    }
    size_t size = 0;
    size_t capacity = INITIAL_BUFFER_CAPACITY;
    output[0] = '\0';

    render_node(root, &output, &size, &capacity);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return output;
}