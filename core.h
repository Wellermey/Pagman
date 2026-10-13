#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <string>
#include <utility>

#define EPS 1e-9

using namespace std;

struct Point {
  double i;
  double j;
};

struct State {
  string map_name = "map1.txt";
  char map[1000][1000];
  double pag_speed;
  Point pag_cord;
  Point curr_route;
  Point next_route;
  bool running;
};

enum class Command { Quit, Up, Down, Left, Right };

class Core {
private:
  State state;
  bool wall(int i, int j) {
    return state.map[i][j] == '*' || state.map[i][j] == '1';
  }

public:
  Core(string map_name) {
    state.map_name = map_name;

    ifstream in(map_name);
    if (in.is_open()) {
      char ch;
      int i = 0, j = 0;
      while (in.get(ch)) {
        if (ch == '\n') {
          state.map[i][j] = ch;
          i++;
          j = 0;
        } else {
          state.map[i][j] = ch;
          j++;
        }
      }
      state.map[i][j] = 'e';
    }
    state.pag_cord = {1, 1};
    state.pag_speed = 1.5;
    state.curr_route = {1, 0};
    state.next_route = {1, 0};
    state.running = true;
  };
  void handle(Command command) {
    switch (command) {
    case Command::Quit:
      state.running = false;
      break;
    case Command::Down:
      state.next_route = {1, 0};
      break;
    case Command::Up:
      state.next_route = {-1, 0};
      break;
    case Command::Left:
      state.next_route = {0, -1};
      break;
    case Command::Right:
      state.next_route = {0, 1};
      break;
    }
  };
  void update(double dt) {
    auto &[map_name, map, pag_speed, pag_cord, curr_route, next_route,
           running] = state;
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
  };
  const State &get_state() const { return state; };
  const bool &running() { return state.running; }
};
