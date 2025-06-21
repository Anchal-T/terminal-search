#ifndef HTML_RENDERER_H
#define HTML_RENDERER_H

#include <stddef.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>


// Original renderer with color ANSI codes
char *html_renderer(const char *html_content);
// Plain text renderer for ncurses
char *html_renderer_plain(const char *html_content);

void render_node(xmlNode *node, char **output, size_t *size, size_t *capacity);
void render_node_plain(xmlNode *node, char **output, size_t *size, size_t *capacity);
void append_to_output(char **output, size_t *size, size_t *capacity, const char *text);
char *clean_text(const char *text);

#endif // HTML_RENDERER_H