#pragma once
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>

#define EPS 1e-9

using namespace std;

inline vector<vector<int>> bfs(int li, int lj, char (&map)[100][100], int sizei,
                               int sizej) {

  vector<vector<int>> d(200, vector<int>(200));
  for (int i = 0; i < sizei; i++)
    for (int j = 0; j < sizej; j++) {

      switch (map[i][j]) {
      case '*':
        d[i][j] = 100000000;
        break;
      case '1':
        d[i][j] = 100000000;
        break;
      case '0':
        d[i][j] = 1000000;
        break;
      case 'm':
        d[i][j] = 1000000;
        break;
      case 'g':
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

struct Editor_state {
  int height;
  int width;
  int selected;
  char cell_type;
  int i;
  int j;
  char map[100][100];
};

class Editor {
  Editor_state state;
  bool _running;
  string name;
  int _error = 0;

  // считает p и g внутри карты прямо по содержимому (без отдельных счётчиков)
  void scan(int &pags, int &ghosts, int &pi, int &pj) {
    pags = ghosts = 0;
    for (int i = 1; i <= state.height; i++)
      for (int j = 1; j <= state.width; j++) {
        if (state.map[i][j] == 'p') {
          pags++;
          pi = i;
          pj = j;
        }
        if (state.map[i][j] == 'g')
          ghosts++;
      }
  }
  // убирает с карты все клетки типа c (p и g должны быть в одном экземпляре)
  void clear_all(char c) {
    for (int i = 0; i < 100; i++)
      for (int j = 0; j < 100; j++)
        if (state.map[i][j] == c)
          state.map[i][j] = '0';
  }
  void resize() {
    for (int i = 0; i < 100; i++)
      for (int j = 0; j < 100; j++) {
        bool inside = i <= state.height + 1 && j <= state.width + 1;
        char &c = state.map[i][j];
        // старая рамка -> пустота; p/g за пределами новой карты -> пустота
        if (c == '*' || (!inside && (c == 'p' || c == 'g')))
          c = '0';
      }
    for (int i = 0; i <= state.height + 1; i++) {
      state.map[i][0] = '*';
      state.map[i][state.width + 1] = '*';
    }
    for (int i = 0; i <= state.width + 1; i++) {
      state.map[0][i] = '*';
      state.map[state.height + 1][i] = '*';
    }
  }
  void update() {
    auto &[height, width, selected, cell_type, i, j, map] = state;
    switch (selected) {
    case 0:
      _running = 0;
      break;
    case 1: {
      int pags, ghosts, pag_i = 0, pag_j = 0;
      scan(pags, ghosts, pag_i, pag_j);
      if (!pags) {
        _error = 'p';
        return;
      }
      if (!ghosts) {
        _error = 'g';
        return;
      }

      // в bfs недостижимая свободная клетка так и остаётся равной 1000000
      auto d = bfs(pag_i, pag_j, map, height + 2, width + 2);
      int unreachable = 0;
      for (int i = 0; i < height + 2; i++)
        for (int j = 0; j < width + 2; j++)
          if (d[i][j] == 1000000)
            unreachable++;
      if (unreachable) {
        _error = 'f';
        return;
      }

      // save
      ofstream out("maps/" + name, ios::binary);
      if (!out.is_open())
        return; // не смогли создать файл — остаёмся в редакторе
      int h = height + 2, w = width + 2;
      out.write(reinterpret_cast<const char *>(&h), sizeof h);
      out.write(reinterpret_cast<const char *>(&w), sizeof w);
      for (int i = 0; i < h; i++)
        out.write(map[i], w);
      _running = 0;
      break;
    }
    case 4:
      cell_type = (!j) ? '1' : (j == 1) ? '0' : (j == 2) ? 'p' : 'g';
      break;
    case 5:
      if (cell_type == 'p' || cell_type == 'g')
        clear_all(cell_type); // p и g на карте один — ставим = переносим
      map[i][j] = cell_type;
      break;
    }
  }

public:
  Editor() {
    _running = true;
    filesystem::create_directories("maps");
    int iname = 0;
    for (const auto &entry : filesystem::directory_iterator("maps")) {
      if (entry.is_regular_file() && entry.path().extension() == ".map") {
        try {
          iname = max(iname, stoi(entry.path().stem().string()));
        } catch (...) { // имя не число — пропускаем
        }
      }
    }
    string num = to_string(iname + 1);
    if (num.size() < 2)
      num = "0" + num;
    name = num + ".map";
    /*
     * 0Quit
     * 1Save
     * 2Height 1 - 97
     * 3Width  1 - 97
     * 4Wall '1' Empty '0' Pagman 'p' Ghost 'g'
     *
     * 5Editor
     * */
    state.height = 1;
    state.width = 1;
    state.selected = 0;
    state.cell_type = '1';
    state.i = 1;
    state.j = 0;
    for (int i = 0; i < 100; i++)
      for (int j = 0; j < 100; j++)
        state.map[i][j] = '0';
    resize();
  }

  void handle(Command command) {
    auto &[height, width, selected, cell_type, i, j, map] = state;
    switch (command) {
    case Command::Down:
      if (selected == 4) {
        selected = 5;
        i = 1;
        j = 1;
      } else {
        if (selected != 5) {
          selected = ((selected + 1) % 6 + 6) % 6;
        } else
          i = ((i) % height + height) % height + 1;
      }
      break;
    case Command::Up:
      if (selected) {
        if (selected != 5)
          selected = ((selected - 1) % 6 + 6) % 6;
        else if (i - 1)
          i--;
        else {
          selected = 4;
          j = 0;
        }
      }
      break;
    case Command::Enter:
      update();
      break;
    case Command::Left:
      switch (selected) {
      case 2:
        if (height - 1 > 0)
          height--;
        resize();
        break;
      case 3:
        if (width - 1 > 0)
          width--;
        resize();
        break;
      case 4:
        j = (j + 3) % 4;
        break;
      case 5:
        j = ((j - 2) % width + width) % width + 1;
        break;
      }
      break;
    case Command::Right:
      switch (selected) {
      case 2:
        if (height + 1 < 98)
          height++;
        resize();
        break;
      case 3:
        if (width + 1 < 98)
          width++;
        resize();
        break;
      case 4:
        j = (j + 1) % 4;
        break;
      case 5:
        j = ((j) % width + width) % width + 1;
        break;
      }
      break;
    case Command::Quit:
      _running = 0;
      break;
    default:
      break;
    }
  }
  const Editor_state &get_state() const { return state; };
  bool running() { return _running; };
  int error() {
    int a = _error;
    _error = 0;
    return a;
  }
};

struct Item {
  string name;
  function<void()> action;
};

struct Changeable {
  double pag_speed;
  double ghost_speed;
  int selected;
};

class Stats {
  Changeable state;
  State_pagman state_pag;
  State_ghost state_ghost;
  int _running;
  void save() {
    state_pag.pag_speed = state.pag_speed;
    state_ghost.ghost_speed = state.ghost_speed;
    ofstream out("state.all", ios::binary);
    out.write(reinterpret_cast<const char *>(&state_pag), sizeof state_pag);
    out.write(reinterpret_cast<const char *>(&state_ghost), sizeof state_ghost);
  }

public:
  Stats() {
    state.selected = 0;
    _running = 1;
    {
      ifstream in("state.all", ios::binary);
      in.read(reinterpret_cast<char *>(&state_pag), sizeof state_pag);
      in.read(reinterpret_cast<char *>(&state_ghost), sizeof state_ghost);
    }
    state.pag_speed = state_pag.pag_speed;
    state.ghost_speed = state_ghost.ghost_speed;
  }

  void handle(Command command) {
    switch (command) {
    case Command::Quit:
      _running = false;
      break;
    case Command::Down:
      state.selected = ((state.selected + 1) % 3 + 3) % 3;
      break;
    case Command::Up:
      state.selected = ((state.selected - 1) % 3 + 3) % 3;
      break;
    case Command::Enter:
      if (state.selected == 2)
        _running = 0;
      break;
    case Command::Left:
      if (state.selected == 0 && state.pag_speed - 0.1 > 0)
        state.pag_speed -= 0.1;
      if (state.selected == 1 && state.ghost_speed - 0.1 > 0)
        state.ghost_speed -= 0.1;
      break;
    case Command::Right:
      if (state.selected == 0 && state.pag_speed + 0.1 < 5)
        state.pag_speed += 0.1;
      if (state.selected == 1 && state.ghost_speed + 0.1 < 6)
        state.ghost_speed += 0.1;
      break;
    default:
      break;
    }
    if (!_running)
      save();
  };

  const Changeable &get_state() const { return state; };
  bool running() { return _running; };
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
        vector<vector<int>> best =
            bfs(lround(state_pag.pag_cord.i), lround(state_pag.pag_cord.j),
                map.map, map.sizei, map.sizej);
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
      if (!in || map.sizei < 3 || map.sizei > 100 || map.sizej < 3 ||
          map.sizej > 100)
        throw runtime_error("bad map header: " + map_name);
      for (int i = 0; i < map.sizei; i++)
        in.read(map.map[i], map.sizej);
      if (!in)
        throw runtime_error("truncated map: " + map_name);
      // карта должна быть обнесена стеной, иначе выход за границы массива
      for (int i = 0; i < map.sizei; i++)
        if (!wall(i, 0) || !wall(i, map.sizej - 1))
          throw runtime_error("map has no border: " + map_name);
      for (int j = 0; j < map.sizej; j++)
        if (!wall(0, j) || !wall(map.sizei - 1, j))
          throw runtime_error("map has no border: " + map_name);
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
