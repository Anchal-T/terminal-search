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

static void render_node(xmlNode *node, char **output, size_t *size, size_t *capacity){
    for(xmlNode *cur = node; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
        const char *name = (const char *)cur->name;
        if(strcasecmp(name, "style") == 0 ||
           strcasecmp(name, "script") == 0 ||
           strcasecmp(name, "head") == 0 ||
           strcasecmp(name, "meta") == 0 ||
           strcasecmp(name, "link") == 0) {
           continue; // Skip these elements
        }

        if(strcasecmp(name, "title") == 0){
            append_to_output(output, size, capacity, BOLD CYAN "=== ");
        }
        else if(strcasecmp(name, "h1") == 0){
            append_to_output(output, size, capacity, BOLD BLUE);
        }
        else if(strcasecmp(name, "h2") == 0){
            append_to_output(output, size, capacity, BOLD GREEN);
        }
        else if(strcasecmp(name, "h3") == 0){
            append_to_output(output, size, capacity, BOLD YELLOW);
        }
        else if(strcasecmp(name, "p") == 0){
            append_to_output(output, size, capacity, "\n");
        }
        else if(strcasecmp(name, "a") == 0){
            append_to_output(output, size, capacity, BLUE);
        }
        else if(strcasecmp(name, "ul") == 0 || strcasecmp(name, "ol") == 0){
            append_to_output(output, size, capacity, RESET);
        }
        else if(strcasecmp(name, "li") == 0){
            append_to_output(output, size, capacity, GREEN);
        }
        else if(strcasecmp(name, "img") == 0){
            append_to_output(output, size, capacity, YELLOW);
        }
        else if(strcasecmp(name, "br") == 0){
            append_to_output(output, size, capacity, "\n");
            continue;
        }
    }
    else if(cur->type == XML_TEXT_NODE) {
        const char *content = (const char *)cur->content;
        char *cleaned = clean_text(content);
        append_to_output(output, size, capacity, cleaned);
        free(cleaned);
    }

    render_node(cur->children, output, size, capacity);
    if (cur->type == XML_ELEMENT_NODE) {
        const char *name = (const char *)cur->name;
                if(strcasecmp(name, "title") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h1") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h2") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h3") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h4") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h5") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "h6") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "p") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "a") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "ul") == 0 || strcasecmp(name, "ol") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
                else if(strcasecmp(name, "li") == 0){
                    append_to_output(output, size, capacity, " === " RESET "\n");
                }
            }
            }
        }
}