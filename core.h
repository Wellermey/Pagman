#pragma once
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <queue>

#define EPS 1e-9

using namespace std;

struct State_menu {
  int selected;
  int size;
  vector<string> menu;
};

struct Point {
  double i, j;
};

struct State_pagman {
  double pag_speed;
  Point pag_cord;
  Point curr_route;
  Point next_route;
  bool running;
  bool win;
  int score;
};

struct State_ghost {
  double ghost_speed;
  Point ghost_cord;
  Point ghost_route;
};

struct Map {
  int sizei;
  int sizej;
  char map[100][100];
};

enum class Command { Quit, Up, Down, Left, Right, Enter };

struct Item {
  string name;
  function<void()> action;
};

class Menu {
  State_menu state;
  vector<Item> items;

public:
  void init(vector<Item> _items) {
    items = _items;
    state.selected = 0;
    state.size = _items.size();
    state.menu.clear();
    for (int i = 0; i < (int)_items.size(); i++)
      state.menu.push_back(_items[i].name);
  }

  void handle(Command command) {
    switch (command) {
    case Command::Quit:
      items[items.size() - 1].action();
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
      items[state.selected].action();
      break;
    default:
      break;
    }
  };

  const State_menu &get_state() const { return state; };
};

class Core {
private:
  State_pagman state_pag;
  State_ghost state_ghost;
  Map map;

  int max_meat;
  bool wall(int i, int j) {
    return map.map[i][j] == '*' || map.map[i][j] == '1';
  }

  void eat(int i, int j) {
    if (map.map[i][j] == 'm') {
      state_pag.score++;
      map.map[i][j] = '0';
      if (state_pag.score == max_meat) {
        state_pag.running = 0;
        state_pag.win = true;
      }
    }
  }

  vector<vector<int>> bfs() {
    int li = lround(state_pag.pag_cord.i);
    int lj = lround(state_pag.pag_cord.j);
    vector<vector<int>> d(1002, vector<int>(1002));
    for (int i = 0; i < map.sizei; i++)
      for (int j = 0; j < map.sizej; j++) {

        switch (map.map[i][j]) {
        case '*':
        case '1':
          d[i][j] = 10000000;
          break;
        case '0':
          d[i][j] = 1000000;
          break;
        case 'm':
          d[i][j] = 1000000;
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
  void kill(double lasti, double lastj, double curri, double currj, double pagi,
            double pagj) {
    if (min(lasti, curri) <= pagi && pagi <= max(lasti, curri) &&
        min(lastj, currj) <= pagj && pagj <= max(lastj, currj)) {
      state_pag.running = false;
      state_pag.win = false;
    }
  }

  void update_ghost(double dt, double last_pagi, double last_pagj) {
    auto &[ghost_speed, ghost_cord, ghost_route] = state_ghost;
    double rem_distance = ghost_speed * dt;

    while (rem_distance > EPS) {
      double lasti = ghost_cord.i, lastj = ghost_cord.j;
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
          ghost_route = {-1, 0};
        else if (best[li + 1][lj] < best[li][lj - 1] &&
                 best[li + 1][lj] < best[li][lj + 1])
          ghost_route = {1, 0};
        else if (best[li][lj - 1] < best[li][lj + 1])
          ghost_route = {0, -1};
        else
          ghost_route = {0, 1};
        if (lround(ghost_cord.i) == lround(state_pag.pag_cord.i) &&
            lround(ghost_cord.j) == lround(state_pag.pag_cord.j))
          ghost_route = {abs(lround(ghost_cord.i) - ghost_cord.i) < EPS ? 0.0
                         : lround(ghost_cord.i) < ghost_cord.i          ? -1.0
                                                                        : 1.0,
                         abs(lround(ghost_cord.j) - ghost_cord.j) < EPS ? 0.0
                         : lround(ghost_cord.j) < ghost_cord.j          ? -1.0
                                                                        : 1.0};
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
      // kill pagman
      kill(lasti, lastj, ghost_cord.i, ghost_cord.j, last_pagi, last_pagj);
      kill(lasti, lastj, ghost_cord.i, ghost_cord.j, state_pag.pag_cord.i,
           state_pag.pag_cord.j);
    }
  }

public:
  Core(string map_name) {
    map_name = "maps/" + map_name;
    max_meat = 0;
    {
      ifstream in("state.all", ios::binary);
      in.read(reinterpret_cast<char *>(&state_pag), sizeof state_pag);
      in.read(reinterpret_cast<char *>(&state_ghost), sizeof state_ghost);
    }
    {
      ifstream in(map_name, ios::binary);
      in.read(reinterpret_cast<char *>(&map.sizei), sizeof(map.sizei));
      in.read(reinterpret_cast<char *>(&map.sizej), sizeof(map.sizej));
      for (int i = 0; i < map.sizei; i++)
        in.read(map.map[i], map.sizej);
      for (int i = 0; i < map.sizei; i++)
        for (int j = 0; j < map.sizej; j++) {
          switch (map.map[i][j]) {
          case '0':
            map.map[i][j] = 'm';
            max_meat++;
            break;
          case 'p':
            state_pag.pag_cord = {(double)i, (double)j};
            map.map[i][j] = '0';
            break;
          case 'g':
            state_ghost.ghost_cord = {(double)i, (double)j};
            map.map[i][j] = '0';
            break;
          }
        }
    }
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
    auto &[pag_speed, pag_cord, curr_route, next_route, running, win, score] =
        state_pag;
    double rem_distance = pag_speed * dt;
    double lasti = pag_cord.i, lastj = pag_cord.j;

    if (next_route.i == -curr_route.i && next_route.j == -curr_route.j) {
      curr_route = next_route;
    }

    while (rem_distance > EPS) {
      double li = pag_cord.i, lj = pag_cord.j;
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
      kill(li, lj, pag_cord.i, pag_cord.j, state_ghost.ghost_cord.i,
           state_ghost.ghost_cord.j);
    }
    update_ghost(dt, lasti, lastj);
  };
  const State_pagman &get_state_pag() const { return state_pag; };
  const State_ghost &get_state_ghost() const { return state_ghost; };
  const Map &get_map() const { return map; };
  const bool &running() { return state_pag.running; }
};
