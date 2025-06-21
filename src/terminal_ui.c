#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../headers/terminal_ui.h"
#include "../headers/http_client.h"
#include "../headers/html_renderer.h"

#define TITLE_COLOR 1
#define URL_COLOR 2
#define SELECTED_COLOR 3
#define BORDER_COLOR 4
#define STATUS_COLOR 5

void init_ui(){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if(has_colors()){
        start_color();
        init_pair(TITLE_COLOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(URL_COLOR, COLOR_CYAN, COLOR_BLACK);
        init_pair(SELECTED_COLOR, COLOR_BLACK, COLOR_WHITE);
        init_pair(BORDER_COLOR, COLOR_GREEN, COLOR_BLACK);
        init_pair(STATUS_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    }
    clear();
    refresh();
}

void cleanup_ui(){
    endwin();
}

void display_search_results(SearchState *state, const char *query){
    int height, width;
    getmaxyx(stdscr, height, width);

    clear();

    attron(COLOR_PAIR(BORDER_COLOR));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(BORDER_COLOR));

    attron(COLOR_PAIR(TITLE_COLOR) | A_BOLD);
    mvprintw(1, 2, "Search query: %s", query);
    attroff(COLOR_PAIR(TITLE_COLOR) | A_BOLD);

    attron(COLOR_PAIR(STATUS_COLOR));
    mvprintw(height - 1, 2, "Use arrow keys to navigate, Enter to open, q to quit");
    attroff(COLOR_PAIR(STATUS_COLOR));

    int results_height = height - 6;
    int start_y = 3;

    for(int i=0; i<state->count && i < results_height; i++){
        int result_index = i + state->scroll_offset;
        if(result_index >= state->count) break;

        SearchResult *result = &state->results[result_index];
        if(i == state->selected){
            attron(COLOR_PAIR(SELECTED_COLOR) | A_BOLD);
            mvprintw(start_y + i, 2, "%s", result->title);
            attroff(COLOR_PAIR(SELECTED_COLOR) | A_BOLD);
        } else {
            attron(COLOR_PAIR(TITLE_COLOR));
            mvprintw(start_y + i, 2, "%s", result->title);
            attroff(COLOR_PAIR(TITLE_COLOR));

            attron(COLOR_PAIR(URL_COLOR));
            mvprintw(start_y + i, 2 + strlen(result->title) + 2, "%s", result->url);
            attroff(COLOR_PAIR(URL_COLOR));
        }
    }
    refresh();
}

int handle_input(SearchState *state){
    int ch = getch();
    int height, width;
    getmaxyx(stdscr, height, width);
    int visible_results = (height - 6) / 3;

    switch (ch) {
        case KEY_UP:
        case 'k':
            if (state->selected > 0) {
                state->selected--;
                if (state->selected < state->scroll_offset) {
                    state->scroll_offset--;
                }
            }
            break;
            
        case KEY_DOWN:
        case 'j':
            if (state->selected < state->count - 1) {
                state->selected++;
                if (state->selected >= state->scroll_offset + visible_results) {
                    state->scroll_offset++;
                }
            }
            break;
            
        case KEY_NPAGE: // Page Down
            state->selected = (state->selected + visible_results < state->count) ? 
                             state->selected + visible_results : state->count - 1;
            state->scroll_offset = (state->selected >= visible_results) ? 
                                  state->selected - visible_results + 1 : 0;
            break;
            
        case KEY_PPAGE: // Page Up
            state->selected = (state->selected - visible_results >= 0) ? 
                             state->selected - visible_results : 0;
            state->scroll_offset = (state->selected < state->scroll_offset) ? 
                                  state->selected : state->scroll_offset;
            break;
            
        case '\n':
        case '\r':
        case KEY_ENTER:
            if (state->count > 0) {
                // Create webpage state and display in UI
                WebpageState *page_state = create_webpage_state(state->results[state->selected].url);
                if (page_state) {
                    // Enter webpage viewing mode
                    while (1) {
                        display_webpage_in_ui(page_state);
                        if (handle_webpage_input(page_state)) {
                            break; // User pressed 'q' to go back
                        }
                    }
                    free_webpage_state(page_state);
                } else {
                    // Show error message briefly
                    int height, width;
                    getmaxyx(stdscr, height, width);
                    attron(COLOR_PAIR(STATUS_COLOR) | A_BOLD);
                    mvprintw(height-3, 2, "Failed to load webpage content");
                    attroff(COLOR_PAIR(STATUS_COLOR) | A_BOLD);
                    refresh();
                    sleep(1);
                }
            }
            break;
            
        case 'q':
        case 'Q':
            return 1; // Quit
            
        default:
            break;
    }
    return 0;
}

void open_url(const char *url) {
    pid_t pid = fork();
    if(pid == 0){
        execl("/usr/bin/xdg-open", "xdg-open", url, (char *)NULL);
        exit(0);
    }
    else if(pid > 0){
        int height, width;
        getmaxyx(stdscr, height, width);
        attron((COLOR_PAIR(STATUS_COLOR) | A_BOLD));
        mvprintw(height - 2, 2, "Opening URL: %s", url);
        attroff((COLOR_PAIR(STATUS_COLOR) | A_BOLD));
        refresh();
        sleep(1);
    }
}

SearchState* parse_duckduckgo_results(cJSON *json){
    SearchState *state = malloc(sizeof(SearchState));
    if(!state) return NULL;

    state->results = NULL;
    state->count = 0;
    state->selected = 0;
    state->scroll_offset = 0;

    cJSON *topics = cJSON_GetObjectItem(json, "RelatedTopics");
    if(!topics || !cJSON_IsArray(topics)){
        return state; // Return empty state instead of NULL
    }

    // Count total results first
    int total_count = 0;
    for (cJSON *item = topics->child; item; item = item->next) {
        cJSON *subtopics = cJSON_GetObjectItem(item, "Topics");
        if (subtopics && cJSON_IsArray(subtopics)) {
            for (cJSON *subitem = subtopics->child; subitem; subitem = subitem->next) {
                cJSON *text = cJSON_GetObjectItem(subitem, "Text");
                cJSON *url = cJSON_GetObjectItem(subitem, "FirstURL");
                if (text && url) total_count++;
            }
        } else {
            cJSON *text = cJSON_GetObjectItem(item, "Text");
            cJSON *url = cJSON_GetObjectItem(item, "FirstURL");
            if (text && url) total_count++;
        }
    }

    if (total_count == 0) return state;

    // Allocate memory for results
    state->results = malloc(sizeof(SearchResult) * total_count);
    if (!state->results) {
        free(state);
        return NULL;
    }

    // Parse and store results
    int index = 0;
    for (cJSON *item = topics->child; item && index < total_count; item = item->next) {
        cJSON *subtopics = cJSON_GetObjectItem(item, "Topics");
        if (subtopics && cJSON_IsArray(subtopics)) {
            for (cJSON *subitem = subtopics->child; subitem && index < total_count; subitem = subitem->next) {
                cJSON *text = cJSON_GetObjectItem(subitem, "Text");
                cJSON *url = cJSON_GetObjectItem(subitem, "FirstURL");
                if (text && url) {
                    state->results[index].title = strdup(text->valuestring);
                    state->results[index].url = strdup(url->valuestring);
                    state->results[index].snippet = NULL;
                    index++;
                }
            }
        } else {
            cJSON *text = cJSON_GetObjectItem(item, "Text");
            cJSON *url = cJSON_GetObjectItem(item, "FirstURL");
            if (text && url) {
                state->results[index].title = strdup(text->valuestring);
                state->results[index].url = strdup(url->valuestring);
                state->results[index].snippet = NULL;
                index++;
            }
        }
    }
    
    state->count = index;
    return state;
}

SearchState* parse_searx_results(cJSON *json) {
    SearchState *state = malloc(sizeof(SearchState));
    if(!state) return NULL;

    state->results = NULL;
    state->count = 0;
    state->selected = 0;
    state->scroll_offset = 0;

    cJSON *results = cJSON_GetObjectItem(json, "results");
    if(!results || !cJSON_IsArray(results)){
        return state; // Return empty state
    }

    int total_count = cJSON_GetArraySize(results);
    if (total_count == 0) return state;

    state->results = malloc(sizeof(SearchResult) * total_count);
    if (!state->results) {
        free(state);
        return NULL;
    }

    int valid_results = 0;
    for (int i = 0; i < total_count; i++) {
        cJSON *item = cJSON_GetArrayItem(results, i);
        cJSON *title = cJSON_GetObjectItem(item, "title");
        cJSON *url = cJSON_GetObjectItem(item, "url");
        cJSON *content = cJSON_GetObjectItem(item, "content");

        if (title && url && title->valuestring && url->valuestring) {
            state->results[valid_results].title = strdup(title->valuestring);
            state->results[valid_results].url = strdup(url->valuestring);
            state->results[valid_results].snippet = content && content->valuestring ? 
                                                   strdup(content->valuestring) : NULL;
            valid_results++;
        }
    }

    state->count = valid_results;
    return state;
}

SearchState* parse_google_results(cJSON *json) {
    SearchState *state = malloc(sizeof(SearchState));
    if(!state) return NULL;

    state->results = NULL;
    state->count = 0;
    state->selected = 0;
    state->scroll_offset = 0;

    printf("Debug: Looking for 'items' in JSON\n");
    cJSON *items = cJSON_GetObjectItem(json, "items");
    if(!items) {
        printf("Debug: No 'items' found, checking for error\n");
        cJSON *error = cJSON_GetObjectItem(json, "error");
        if (error) {
            cJSON *message = cJSON_GetObjectItem(error, "message");
            if (message && message->valuestring) {
                printf("Google API Error: %s\n", message->valuestring);
            }
        }
        return state; // Return empty state
    }
    
    if (!cJSON_IsArray(items)) {
        printf("Debug: 'items' is not an array\n");
        return state; // Return empty state
    }

    int total_count = cJSON_GetArraySize(items);
    printf("Debug: Found %d items in response\n", total_count);
    if (total_count == 0) return state;

    state->results = malloc(sizeof(SearchResult) * total_count);
    if (!state->results) {
        free(state);
        return NULL;
    }

    int valid_results = 0;
    for (int i = 0; i < total_count; i++) {
        cJSON *item = cJSON_GetArrayItem(items, i);
        cJSON *title = cJSON_GetObjectItem(item, "title");
        cJSON *link = cJSON_GetObjectItem(item, "link");
        cJSON *snippet = cJSON_GetObjectItem(item, "snippet");

        if (title && link && title->valuestring && link->valuestring) {
            state->results[valid_results].title = strdup(title->valuestring);
            state->results[valid_results].url = strdup(link->valuestring);
            state->results[valid_results].snippet = snippet && snippet->valuestring ? 
                                                   strdup(snippet->valuestring) : NULL;
            valid_results++;
        }
    }

    state->count = valid_results;
    return state;
}

void free_search_state(SearchState *state) {
    if (!state) return;
    for (int i = 0; i < state->count; i++) {
        free(state->results[i].title);
        free(state->results[i].url);
        free(state->results[i].snippet);
    }
    free(state->results);
    free(state);
}

void display_webpage_content(const char *url) {
    if (!url) return;
    
    // Temporarily exit ncurses mode
    def_prog_mode();
    endwin();
    
    printf("Fetching: %s\n", url);
    
    // Use your existing HTTP client and HTML renderer
    HTTPResponse response = {NULL, 0};
    if (http_search(url, "", &response) == 0) {
        char *rendered_content = html_renderer(response.data);
        if (rendered_content) {
            printf("%s\n", rendered_content);
            free(rendered_content);
        } else {
            printf("Failed to render webpage content.\n");
        }
        http_response_free(&response);
    } else {
        printf("Failed to fetch webpage.\n");
    }
    
    printf("\nPress Enter to return to search results...");
    getchar();
    
    // Resume ncurses mode
    reset_prog_mode();
    refresh();
}

WebpageState* create_webpage_state(const char *url) {
    if (!url) return NULL;
    
    // Fetch webpage content
    HTTPResponse response = {NULL, 0};
    if (http_search(url, "", &response) != 0) {
        return NULL;
    }
    
    // Render HTML to text
    char *rendered_content = html_renderer(response.data);
    http_response_free(&response);
    
    if (!rendered_content) {
        return NULL;
    }
    
    // Create webpage state
    WebpageState *state = malloc(sizeof(WebpageState));
    if (!state) {
        free(rendered_content);
        return NULL;
    }
    
    state->url = strdup(url);
    state->title = strdup("Webpage Content");
    state->scroll_offset = 0;
    
    // Count lines first
    int line_count = 1; // Start with 1 for the last line
    for (char *p = rendered_content; *p; p++) {
        if (*p == '\n') line_count++;
    }
    
    // Allocate lines array
    state->lines = malloc(sizeof(char*) * line_count);
    state->line_count = 0;
    
    // Split content into lines
    char *content_copy = strdup(rendered_content);
    char *line = strtok(content_copy, "\n");
    while (line != NULL && state->line_count < line_count) {
        state->lines[state->line_count] = strdup(line);
        state->line_count++;
        line = strtok(NULL, "\n");
    }
    
    free(content_copy);
    free(rendered_content);
    return state;
}

void display_webpage_in_ui(WebpageState *page_state) {
    int height, width;
    getmaxyx(stdscr, height, width);
    
    clear();
    
    // Draw border
    attron(COLOR_PAIR(BORDER_COLOR));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(BORDER_COLOR));
    
    // Title bar
    attron(COLOR_PAIR(TITLE_COLOR) | A_BOLD);
    mvprintw(1, 2, "Viewing: %.*s", width-10, page_state->url);
    attroff(COLOR_PAIR(TITLE_COLOR) | A_BOLD);
    
    // Status bar
    attron(COLOR_PAIR(STATUS_COLOR));
    mvprintw(height-2, 2, "↑/↓: Scroll | q: Back to search | Page %d/%d", 
             page_state->scroll_offset/10 + 1, 
             (page_state->line_count + 9)/10);
    attroff(COLOR_PAIR(STATUS_COLOR));
    
    // Content area
    int content_height = height - 5;
    int start_y = 3;
    
    for (int i = 0; i < content_height && i + page_state->scroll_offset < page_state->line_count; i++) {
        int line_index = i + page_state->scroll_offset;
        mvprintw(start_y + i, 2, "%.*s", width-4, page_state->lines[line_index]);
    }
    
    refresh();
}

int handle_webpage_input(WebpageState *page_state) {
    int ch = getch();
    int height, width;
    getmaxyx(stdscr, height, width);
    int content_height = height - 5;
    
    switch (ch) {
        case KEY_UP:
        case 'k':
            if (page_state->scroll_offset > 0) {
                page_state->scroll_offset--;
            }
            break;
            
        case KEY_DOWN:
        case 'j':
            if (page_state->scroll_offset + content_height < page_state->line_count) {
                page_state->scroll_offset++;
            }
            break;
            
        case KEY_NPAGE: // Page Down
            page_state->scroll_offset += content_height;
            if (page_state->scroll_offset + content_height >= page_state->line_count) {
                page_state->scroll_offset = page_state->line_count - content_height;
                if (page_state->scroll_offset < 0) page_state->scroll_offset = 0;
            }
            break;
            
        case KEY_PPAGE: // Page Up
            page_state->scroll_offset -= content_height;
            if (page_state->scroll_offset < 0) {
                page_state->scroll_offset = 0;
            }
            break;
            
        case 'q':
        case 'Q':
        case KEY_BACKSPACE:
        case 27: // ESC
            return 1; // Go back to search
            
        default:
            break;
    }
    
    return 0; // Continue viewing webpage
}

void free_webpage_state(WebpageState *state) {
    if (!state) return;
    
    for (int i = 0; i < state->line_count; i++) {
        free(state->lines[i]);
    }
    free(state->lines);
    free(state->title);
    free(state->url);
    free(state);
}
