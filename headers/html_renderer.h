#ifndef HTML_RENDERER_H
#define HTML_RENDERER_H

#include <stddef.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>


char *html_renderer(const char *html_content);
static void render_node(xmlNode *node, char **output, size_t *size, size_t *capacity);
static void append_to_output(char **output, size_t *size, size_t *capacity, const char *text);
static char *clean_text(const char *text);

#endif