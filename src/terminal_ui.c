#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../headers/terminal_ui.h"

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
                open_url(state->results[state->selected].url);
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

