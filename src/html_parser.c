#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include "html_parser.h"

char *html_parse(const char *html_content) {
    if(!html_content) return NULL;

    htmlDocPtr doc = htmlReadMemory(html_content, strlen(html_content), NULL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if(!doc) {
        return NULL; // Failed to parse HTML
    }
    xmlNode *root = xmlDocGetRootElement(doc);
    if(!root) {
        xmlFreeDoc(doc);
        return NULL; // No root element found
    }

    xmlChar *xml_content = xmlNodeGetContent(root);
    char *result = xml_content ? strdup((const char *)xml_content) : NULL;
    xmlFree(xml_content);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}

int main(){
    const char *html = "<html><body><h1>Hello, World!</h1></body></html>";
    char *text = html_parse(html);
    if(text) {
        printf("Parsed text: %s\n", text);
        free(text);
    } else {
        printf("Failed to parse HTML.\n");
    }
    return 0;
}