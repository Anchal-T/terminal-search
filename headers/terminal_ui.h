#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <ncurses.h>
#include <cjson/cJSON.h>

typedef struct{
    char *title;
    char *url;
    char *snippet;
} SearchResult;

typedef struct{
    SearchResult *results;
    int count;
    int selected;
    int scroll_offset;
} SearchState;

typedef struct {
    char **lines;
    int line_count;
    int scroll_offset;
    char *title;
    char *url;
} WebpageState;

void init_ui();
void cleanup_ui();
void display_search_results(SearchState *state, const char *query);
void display_webpage_in_ui(WebpageState *page_state);
int handle_input(SearchState *state);
int handle_webpage_input(WebpageState *page_state);
void open_url(const char *url);
SearchState* parse_duckduckgo_results(cJSON *json);
SearchState* parse_searx_results(cJSON *json);
SearchState* parse_google_results(cJSON *json);
void free_search_state(SearchState *state);
void free_webpage_state(WebpageState *state);
WebpageState* create_webpage_state(const char *url);
void display_webpage_content(const char *url);

#endif // TERMINAL_UI_H