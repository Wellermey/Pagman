#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define EPS 1e-9

using namespace std;

struct Point {
  double i;
  double j;
};

struct State_menu {
  int selected;
  int size;
  vector<string> menu;
};

struct State_pagman {
  string map_name = "map1.txt";
  char map[1000][1000];
  double pag_speed;
  Point pag_cord;
  Point curr_route;
  Point next_route;
  bool running;
  int score;
};

struct State_ghost {
  double ghost_speed;
  Point ghost_cord;
  Point ghost_route;
};

enum class Command { Quit, Up, Down, Left, Right, Enter };

class Menu {
private:
  State_menu state;
  bool _running;
  bool _pagman;
  int _type;
  string map_name;
  vector<string> maps;

  void update() {
    switch (_type) {
    case 'i':
      if (state.selected == 0)
        _pagman = true;
      else if (state.selected == 1) {
        _type = 'm';
        state.selected = 0;
        state.menu = {};
        state.size = maps.size() + 1;
        for (int i = 1; i <= (int)maps.size(); i++) {
          state.menu.push_back(to_string(i) + ") " + maps[i - 1]);
        }
        state.menu.push_back(to_string(state.size) + ") Back");
      }
      if (state.selected == 2)
        _running = false;
      break;
    case 'm':
      if (state.selected != state.size - 1)
        map_name = maps[state.selected];
      state.selected = 0;
      state.size = 3;
      state.menu = {"1) Play", "2) Change map", "3) Quit"};
      _type = 'i';
      break;
    }
  };

public:
  Menu() {
    state.selected = 0;
    state.menu = {"1) Play", "2) Change map", "3) Quit"};
    state.size = 3;
    _running = true;
    _pagman = false;
    _type = 'i';

    for (const auto &entry : filesystem::directory_iterator(".")) {
      if (entry.is_regular_file() && entry.path().extension() == ".txt") {
        maps.push_back(entry.path().filename().string());
      }
    }
    map_name = maps[0];
  };
  void handle(Command command) {
    switch (command) {
    case Command::Quit:
      _running = false;
      break;
    case Command::Down:
      state.selected =
          ((state.selected + 1) % state.size + state.size) % state.size;
      break;
    case Command::Up:
      state.selected =
          ((state.selected - 1) % state.size + state.size) % state.size;
      break;
    case Command::Enter:
      update();
      break;
    default:
      break;
    }
  };
  bool running() { return _running; };
  bool pagman() { return _pagman; };
  const State_menu &get_state() const { return state; };
  string name() { return map_name; };
};

class Core {
private:
  State_pagman state_pag;
  State_ghost state_ghost;
  int max_meat;
  bool wall(int i, int j) {
    return state_pag.map[i][j] == '*' || state_pag.map[i][j] == '1';
  }
  void eat(int i, int j) {
    if (state_pag.map[i][j] == 'm') {
      state_pag.score++;
      state_pag.map[i][j] = '0';
      if (state_pag.score == max_meat)
        state_pag.running = 0;
    }
  }
  vector<vector<int>> bfs() {
    int li = lround(state_pag.pag_cord.i);
    int lj = lround(state_pag.pag_cord.j);
    vector<vector<int>> d(1000, vector<int>(1000));
    int i = 0, j = 0;
    while (state_pag.map[i][j] != 'e') {
      switch (state_pag.map[i][j]) {
      case '*':
      case '1':
        d[i][j] = 10000000;
        j++;
        break;
      case '0':
        d[i][j] = 1000000;
        j++;
        break;
      case 'm':
        d[i][j] = 1000000;
        j++;
        break;
      case '\n':
        i++;
        j = 0;
        break;
      }
    }
    d[li][lj] = 0;
    queue<pair<int, int>> queue;
    queue.push({li, lj});
    while (queue.size()) {
      auto u = queue.front();
      queue.pop();
      if (d[u.first - 1][u.second] == 1000000) {
        d[u.first - 1][u.second] = d[u.first][u.second] + 1;
        queue.push({u.first - 1, u.second});
      }
      if (d[u.first + 1][u.second] == 1000000) {
        d[u.first + 1][u.second] = d[u.first][u.second] + 1;
        queue.push({u.first + 1, u.second});
      }
      if (d[u.first][u.second - 1] == 1000000) {
        d[u.first][u.second - 1] = d[u.first][u.second] + 1;
        queue.push({u.first, u.second - 1});
      }
      if (d[u.first][u.second + 1] == 1000000) {
        d[u.first][u.second + 1] = d[u.first][u.second] + 1;
        queue.push({u.first, u.second + 1});
      }
    }
    return d;
  }
  void update_ghost(double dt) {
    auto &[ghost_speed, ghost_cord, ghost_route] = state_ghost;
    double rem_distance = ghost_speed * dt;

    while (rem_distance > EPS) {
      if (fabs(ghost_cord.i - round(ghost_cord.i)) < EPS &&
          fabs(ghost_cord.j - round(ghost_cord.j)) < EPS) {
        ghost_cord.i = round(ghost_cord.i);
        ghost_cord.j = round(ghost_cord.j);
        int li = lround(ghost_cord.i);
        int lj = lround(ghost_cord.j);
        vector<vector<int>> best = bfs();
        if (best[li - 1][lj] < best[li + 1][lj] &&
            best[li - 1][lj] < best[li][lj - 1] &&
            best[li - 1][lj] < best[li][lj + 1])
          state_ghost.ghost_route = {-1, 0};
        else if (best[li + 1][lj] < best[li][lj - 1] &&
                 best[li + 1][lj] < best[li][lj + 1])
          state_ghost.ghost_route = {1, 0};
        else if (best[li][lj - 1] < best[li][lj + 1])
          state_ghost.ghost_route = {0, -1};
        else
          state_ghost.ghost_route = {0, 1};
      }
      double &curr_cord = ghost_route.i ? ghost_cord.i : ghost_cord.j;
      double target = (ghost_route.i + ghost_route.j) > 0 ? floor(curr_cord) + 1
                                                          : ceil(curr_cord) - 1;
      double last_cord = curr_cord;
      if (fabs(target - curr_cord) < EPS)
        curr_cord = target;
      else
        curr_cord += (ghost_route.i + ghost_route.j) *
                     min(rem_distance, fabs(target - curr_cord));
      rem_distance -= min(rem_distance, fabs(target - last_cord));
    }
  }

public:
  Core(string map_name) {
    state_pag.map_name = map_name;
    max_meat = 0;

    ifstream in(map_name);
    if (in.is_open()) {
      char ch;
      int i = 0, j = 0;
      while (in.get(ch)) {
        if (ch == '\n') {
          state_pag.map[i][j] = ch;
          i++;
          j = 0;
        } else if (ch == '0') {
          state_pag.map[i][j] = 'm';
          max_meat++;
          j++;
        } else {
          state_pag.map[i][j] = ch;
          j++;
        }
      }
      state_pag.map[i][j] = 'e';
    }
    state_pag.pag_cord = {1, 1};
    state_pag.pag_speed = 1.5;
    state_pag.curr_route = {1, 0};
    state_pag.next_route = {1, 0};
    state_pag.running = true;
    state_pag.score = 0;
    // ghosts
    state_ghost.ghost_cord = {3, 2};
    state_ghost.ghost_route = {0, 1};
    state_ghost.ghost_speed = 1;
  };
  void handle(Command command) {
    switch (command) {
    case Command::Quit:
      state_pag.running = false;
      break;
    case Command::Down:
      state_pag.next_route = {1, 0};
      break;
    case Command::Up:
      state_pag.next_route = {-1, 0};
      break;
    case Command::Left:
      state_pag.next_route = {0, -1};
      break;
    case Command::Right:
      state_pag.next_route = {0, 1};
      break;
    default:
      break;
    }
  };
  void update(double dt) {
    auto &[map_name, map, pag_speed, pag_cord, curr_route, next_route, running,
           score] = state_pag;
    double rem_distance = pag_speed * dt;

    if (next_route.i == -curr_route.i && next_route.j == -curr_route.j) {
      curr_route = next_route;
    }

    while (rem_distance > EPS) {
      if (fabs(pag_cord.i - round(pag_cord.i)) < EPS &&
          fabs(pag_cord.j - round(pag_cord.j)) < EPS) {
        pag_cord.i = round(pag_cord.i);
        pag_cord.j = round(pag_cord.j);
        int int_i = lround(pag_cord.i);
        int int_j = lround(pag_cord.j);
        eat(int_i, int_j);
        if (!wall(int_i + next_route.i, int_j + next_route.j))
          curr_route = next_route;

        if (wall(int_i + curr_route.i, int_j + curr_route.j)) {
          curr_route = {0, 0};
          return;
        }
      }

      double &curr_cord = curr_route.i ? pag_cord.i : pag_cord.j;
      double target = (curr_route.i + curr_route.j) > 0 ? floor(curr_cord) + 1
                                                        : ceil(curr_cord) - 1;
      double last_cord = curr_cord;
      if (fabs(target - curr_cord) < EPS)
        curr_cord = target;
      else
        curr_cord += (curr_route.i + curr_route.j) *
                     min(rem_distance, fabs(target - curr_cord));
      rem_distance -= min(rem_distance, fabs(target - last_cord));
    }

    update_ghost(dt);
  };
  const State_pagman &get_state_pag() const { return state_pag; };
  const State_ghost &get_state_ghost() const { return state_ghost; };
  const bool &running() { return state_pag.running; }
};
