#include "core.h"
#include "front.h"
#include <cmath>
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
                      const State_ghost &state_ghost) {
  erase();
  bkgd(COLOR_PAIR(4));
  int i = 0, j = 0;
  int len = 0;
  while (state_pag.map[i][j] != 'e') {
    switch (state_pag.map[i][j]) {
    case '*':
    case '1':
      attron(COLOR_PAIR(1));
      addch('#');
      addch('#');
      attroff(COLOR_PAIR(1));
      j++;
      break;
    case '0':
      attron(COLOR_PAIR(2));
      addch(' ');
      addch(' ');
      attroff(COLOR_PAIR(2));
      j++;
      break;
    case 'm':
      attron(COLOR_PAIR(4));
      addstr("• ");
      attroff(COLOR_PAIR(4));
      j++;
      break;
    case '\n':
      addch('\n');
      i++;
      len = 2 * j;
      j = 0;
      break;
    }
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
  mvprintw(0, len, "Score: %i", state_pag.score);
  attroff(COLOR_PAIR(4));

  refresh();
}
void Frontend::render(const State_menu &state) {
  erase();
  bkgd(COLOR_PAIR(4));
  for (int i = 0; i < (int)state.menu.size(); i++) {
    if (i == state.selected)
      attron(COLOR_PAIR(5));
    else
      attron(COLOR_PAIR(4));
    mvprintw(2 + 2 * i, 2, "%s", state.menu[i].c_str());
    if (i == state.selected)
      attroff(COLOR_PAIR(5));
    else
      attroff(COLOR_PAIR(4));
  }

  refresh();
}

Frontend::~Frontend() { endwin(); }
