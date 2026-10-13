#include "core.h"
#include "front.h"
#include <cmath>
#include <ncurses.h>
#include <optional>

#define MY_RED 30
#define MY_PACMAN 31
#define MY_EMPTY 32

Frontend::Frontend() {
  initscr();
  curs_set(0);
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  start_color();

  init_color(MY_RED, 952, 545, 658);
  init_pair(1, MY_RED, MY_RED);
  init_color(MY_EMPTY, 117, 117, 180);
  init_pair(2, MY_EMPTY, MY_EMPTY);
  init_color(MY_PACMAN, 1000, 716, 540);
  init_pair(3, MY_PACMAN, MY_EMPTY);
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
  default:
    return std::nullopt;
  }
}
void Frontend::render(const State &state) {
  erase();

  int i = 0, j = 0;
  while (state.map[i][j] != 'e') {
    switch (state.map[i][j]) {
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
    case '\n':
      addch('\n');
      i++;
      j = 0;
      break;
    }
  }

  attron(COLOR_PAIR(3));
  mvaddch(lround(state.pag_cord.i), 2 * lround(state.pag_cord.j), '(');
  mvaddch(lround(state.pag_cord.i), 2 * lround(state.pag_cord.j) + 1, ')');
  attroff(COLOR_PAIR(3));
}
Frontend::~Frontend() { endwin(); }
