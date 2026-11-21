#include "core.h"
#include "front.h"
#include <cmath>
#include <string>
#include <vector>
#if __has_include(<ncurses.h>)
#include <ncurses.h>
#else
#include <curses.h>
#endif
#include <optional>

#define MY_RED 30
#define MY_PACMAN 31
#define MY_EMPTY 32
#define MY_TEXT 33
#define MY_GREEN 34

Frontend::Frontend() {
  setlocale(LC_ALL, "");
  initscr();
  curs_set(0);
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  start_color();

  init_color(MY_RED, 952, 545, 658);
  init_pair(1, MY_RED, MY_RED);
  init_color(MY_EMPTY, 118, 118, 180);
  init_pair(2, MY_EMPTY, MY_EMPTY);
  init_color(MY_PACMAN, 1000, 716, 540);
  init_pair(3, MY_PACMAN, MY_EMPTY);
  init_color(MY_TEXT, 804, 839, 957);
  init_pair(4, MY_TEXT, MY_EMPTY);
  init_pair(5, MY_EMPTY, MY_TEXT);
  init_color(MY_GREEN, 651, 890, 631);
  init_pair(6, MY_EMPTY, MY_GREEN);
  init_pair(7, MY_TEXT, MY_RED);
}
std::optional<Command> Frontend::Input() {
  int ch = getch();
  switch (ch) {
  case 'q':
  case 'Q':
    return Command::Quit;
  case KEY_UP:
    return Command::Up;
  case KEY_DOWN:
    return Command::Down;
  case KEY_LEFT:
    return Command::Left;
  case KEY_RIGHT:
    return Command::Right;
  case '\n':
    return Command::Enter;
  default:
    return std::nullopt;
  }
}
void Frontend::render(const State_pagman &state_pag,
                      const State_ghost &state_ghost, const Map &map) {
  erase();
  bkgd(COLOR_PAIR(4));
  for (int i = 0; i < map.sizei; i++) {
    for (int j = 0; j < map.sizej; j++) {
      switch (map.map[i][j]) {
      case '*':
      case '1':
        attron(COLOR_PAIR(1));
        addch('#');
        addch('#');
        attroff(COLOR_PAIR(1));
        break;
      case '0':
        attron(COLOR_PAIR(2));
        addch(' ');
        addch(' ');
        attroff(COLOR_PAIR(2));
        break;
      case 'm':
        attron(COLOR_PAIR(4));
        addstr("• ");
        attroff(COLOR_PAIR(4));
        break;
      }
    }
    addch('\n');
  }
  // pagman
  attron(COLOR_PAIR(3));
  mvaddch(lround(state_pag.pag_cord.i), 2 * lround(state_pag.pag_cord.j), '(');
  mvaddch(lround(state_pag.pag_cord.i), 2 * lround(state_pag.pag_cord.j) + 1,
          ')');
  attroff(COLOR_PAIR(3));
  // ghost
  attron(COLOR_PAIR(4));
  mvaddch(lround(state_ghost.ghost_cord.i),
          2 * lround(state_ghost.ghost_cord.j), '/');
  mvaddch(lround(state_ghost.ghost_cord.i),
          2 * lround(state_ghost.ghost_cord.j) + 1, '\\');
  attroff(COLOR_PAIR(4));
  // score
  attron(COLOR_PAIR(4));
  mvprintw(0, 2 * map.sizej, "Score: %i", state_pag.score);
  attroff(COLOR_PAIR(4));

  refresh();
}
void Frontend::render(const State_menu &state, string map_name) {
  erase();
  bkgd(COLOR_PAIR(4));
  for (int i = 0; i < (int)state.menu.size(); i++) {
    if (i == state.selected)
      attron(COLOR_PAIR(5));
    else
      attron(COLOR_PAIR(4));
    if (state.menu[i] == map_name)
      mvprintw(1 + 2 * i, 2, "> %s", state.menu[i].c_str());
    else
      mvprintw(1 + 2 * i, 2, "%s", state.menu[i].c_str());
    if (i == state.selected)
      attroff(COLOR_PAIR(5));
    else
      attroff(COLOR_PAIR(4));
  }

  refresh();
}

void Frontend::render(const Changeable stats) {
  erase();
  bkgd(COLOR_PAIR(4));
  attron(COLOR_PAIR(4));
  mvprintw(1, 2, "Pagman speed %.1f", stats.pag_speed);
  mvprintw(3, 2, "Ghost speed %.1f", stats.ghost_speed);
  mvprintw(5, 2, "Back");
  attroff(COLOR_PAIR(4));

  mvchgat(1 + 2 * stats.selected, 2, 16, A_NORMAL, 5, NULL);
  refresh();
}

void Frontend::render(const Editor_state state) {
  erase();
  bkgd(COLOR_PAIR(4));

  attron(COLOR_PAIR(state.selected == 0 ? 5 : 4));
  mvprintw(1, 2, "Quit");
  attroff(COLOR_PAIR(state.selected == 0 ? 5 : 4));

  attron(COLOR_PAIR(state.selected == 1 ? 5 : 4));
  mvprintw(3, 2, "Save");
  attroff(COLOR_PAIR(state.selected == 1 ? 5 : 4));

  attron(COLOR_PAIR(state.selected == 2 ? 5 : 4));
  mvprintw(5, 2, "Height: %d", state.height);
  attroff(COLOR_PAIR(state.selected == 2 ? 5 : 4));

  attron(COLOR_PAIR(state.selected == 3 ? 5 : 4));
  mvprintw(7, 2, "Width:  %d", state.width);
  attroff(COLOR_PAIR(state.selected == 3 ? 5 : 4));

  const char *names[] = {"Wall", "Empty", "Pagman", "Ghost"};
  const char types[] = {'1', '0', 'p', 'g'};
  int px = 2;
  for (int k = 0; k < 4; k++) {
    bool active = state.cell_type == types[k];
    bool cursor = state.selected == 4 && state.j == k;
    string s = string(active ? "[" : " ") + names[k] + (active ? "]" : " ");
    attron(COLOR_PAIR(cursor ? 5 : 4));
    mvprintw(9, px, "%s", s.c_str());
    attroff(COLOR_PAIR(cursor ? 5 : 4));
    px += (int)s.size() + 1;
  }
  const int top = 11;
  int maxi, maxj;
  getmaxyx(stdscr, maxi, maxj);
  int rows = state.height + 2, cols = state.width + 2;
  int view_h = maxi - top, view_w = maxj / 2;
  auto calc_off = [](int cur, int view, int total) {
    return max(0, min(cur - view / 2, total - view));
  };
  int off_i = state.selected == 5 ? calc_off(state.i, view_h, rows) : 0;
  int off_j = state.selected == 5 ? calc_off(state.j, view_w, cols) : 0;

  for (int y = 0; y < min(rows, view_h); y++)
    for (int x = 0; x < min(cols, view_w); x++) {
      int i = y + off_i, j = x + off_j;
      bool cursor = state.selected == 5 && i == state.i && j == state.j;
      const char *glyph = "  ";
      int pair = 4;
      switch (state.map[i][j]) {
      case '*':
      case '1':
        glyph = "##";
        pair = 1;
        break;
      case 'p':
        glyph = "()";
        pair = 3;
        break;
      case 'g':
        glyph = "/\\";
        pair = 4;
        break;
      }
      if (cursor)
        pair = 5;
      attron(COLOR_PAIR(pair));
      mvaddstr(top + y, 2 + 2 * x, glyph);
      attroff(COLOR_PAIR(pair));
    }
  refresh();
}

void Frontend::render(int err) {
  erase();
  int maxi, maxj;
  getmaxyx(stdscr, maxi, maxj);
  bkgd(COLOR_PAIR(7));
  attron(COLOR_PAIR(7));

  vector<string> text;
  string subtitle = "";
  int width = 0;

  switch (err) {
  case 'p': // No Pagman
    width = 49;
    text = {"█   █  ███    ████   ███   ████ █   █  ███  █   █",
            "██  █ █   █   █   █ █   █ █     ██ ██ █   █ ██  █",
            "█ █ █ █   █   ████  █████ █  ██ █ █ █ █████ █ █ █",
            "█  ██ █   █   █     █   █ █   █ █   █ █   █ █  ██",
            "█   █  ███    █     █   █  ████ █   █ █   █ █   █"};
    break;

  case 'g': // No Ghost
    width = 43;
    text = {"█   █  ███     ████ █   █  ███   ████ █████",
            "██  █ █   █   █     █   █ █   █ █       █  ",
            "█ █ █ █   █   █  ██ █████ █   █  ███    █  ",
            "█  ██ █   █   █   █ █   █ █   █     █   █  ",
            "█   █  ███     ████ █   █  ███  ████    █  "};
    break;

  case 'f': // Pagman must reach all the cells
    width = 37;
    text = {"█   █  ███    ████   ███  █████ █   █",
            "██  █ █   █   █   █ █   █   █   █   █",
            "█ █ █ █   █   ████  █████   █   █████",
            "█  ██ █   █   █     █   █   █   █   █",
            "█   █  ███    █     █   █   █   █   █"};
    subtitle = "Pagman must reach all the cells";
    break;
  }

  int starti = (maxi - static_cast<int>(text.size())) / 2;
  int startj = (maxj - width) / 2;

  for (size_t i = 0; i < text.size(); i++) {
    mvprintw(starti + i, startj, "%s", text[i].c_str());
  }

  if (!subtitle.empty()) {
    int sub_j = (maxj - static_cast<int>(subtitle.length())) / 2;
    mvprintw(starti + text.size() + 1, sub_j, "%s", subtitle.c_str());
  }

  attroff(COLOR_PAIR(7));
  refresh();
}

void Frontend::render(bool win, int score) {
  erase();
  int maxi, maxj;
  getmaxyx(stdscr, maxi, maxj);
  if (win) {
    int starti = (maxi - 5) / 2, startj = (maxj - 15) / 2;
    bkgd(COLOR_PAIR(6));
    attron(COLOR_PAIR(6));
    vector<string> text = {"█   █ ███ █   █", "█   █  █  ██  █",
                           "█ █ █  █  █ █ █", "██ ██  █  █  ██",
                           "█   █ ███ █   █"};
    for (int i = 0; i < 5; i++)
      mvprintw(starti + i, startj, "%s", text[i].c_str());
    mvprintw(starti + 5, (maxj - 7 - to_string(score).size()) / 2, "Score: %s",
             to_string(score).c_str());
    attroff(COLOR_PAIR(6));
  } else {
    int starti = (maxi - 5) / 2, startj = (maxj - 24) / 2;
    bkgd(COLOR_PAIR(7));
    attron(COLOR_PAIR(7));
    vector<string> text = {"█      ███   ████ █████", "█     █   █ █     █    ",
                           "█     █   █  ███  ████ ", "█     █   █     █ █    ",
                           "█████  ███  ████  █████"};
    for (int i = 0; i < 5; i++)
      mvprintw(starti + i, startj, "%s", text[i].c_str());
    mvprintw(starti + 5, (maxj - 7 - to_string(score).size()) / 2, "Score: %s",
             to_string(score).c_str());
    attroff(COLOR_PAIR(7));
  }

  refresh();
}

Frontend::~Frontend() { endwin(); }
