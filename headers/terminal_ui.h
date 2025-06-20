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

void init_ui();
void cleanup_ui();
void display_search_results(SearchState *state, const char *query);
int handle_input(SearchState *state);
void open_url(const char *url);
SearchState* parse_duckduckgo_results(cJSON *json);
void free_search_state(SearchState *state);

#endif // TERMINAL_UI_H