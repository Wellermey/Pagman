
#include "core.h"
#include "front.h"
#include <algorithm>
#include <chrono>
#include <optional>
#include <thread>

using namespace std;
using namespace chrono;

vector<string> get_maps() {
  vector<string> maps;
  filesystem::create_directories("maps");
  for (const auto &entry : filesystem::directory_iterator("maps")) {
    if (entry.is_regular_file() && entry.path().extension() == ".map") {
      maps.push_back(entry.path().filename().string());
    }
  }
  sort(maps.begin(), maps.end()); // порядок directory_iterator не определён
  return maps;
}

void init_menu(int &menu_type, Menu &menu, int &what_run, string &map_name,
               vector<string> maps) {
  if (menu_type == 'i')
    menu.init({
        {"Play", [&what_run] { what_run = 'p'; }},
        {"Change map", [&menu_type] { menu_type = 'm'; }},
        {"Create map", [&what_run] { what_run = 'e'; }},
        {"Change stats", [&what_run] { what_run = 's'; }},
        {"Quit", [&what_run] { what_run = 0; }},
    });
  else if (menu_type == 'm') {
    vector<Item> v;
    for (int i = 0; i < (int)maps.size(); i++)
      v.push_back({maps[i], [&map_name, &menu_type, name = maps[i]] {
                     map_name = name;
                     menu_type = 'i';
                   }});
    v.push_back({"Back", [&menu_type] { menu_type = 'i'; }});
    menu.init(v);
  }

  menu_type = 0;
}

int main() {

  Menu menu;
  Frontend front;
  int menu_type = 'i';
  string map_name = "01.map";
  int what_run = 'm';
  vector<string> maps = get_maps();
  if (!maps.empty() && find(maps.begin(), maps.end(), map_name) == maps.end())
    map_name = maps[0];

  while (what_run) {
    if (menu_type) {
      init_menu(menu_type, menu, what_run, map_name, maps);
    }
    auto command = front.Input();
    if (command.has_value())
      menu.handle(*command);
    if (what_run == 'm')
      front.render(menu.get_state(), map_name);
    if (what_run == 's') {
      Stats stats;
      while (stats.running()) {
        auto command = front.Input();
        if (command.has_value())
          stats.handle(*command);
        front.render(stats.get_state());
      }
      what_run = 'm';
    }
    if (what_run == 'e') {
      Editor editor;
      int a = 0;
      while (editor.running()) {
        auto command = front.Input();
        if (command.has_value())
          editor.handle(*command);
        a = editor.error();
        if (!a)
          front.render(editor.get_state());
        else {
          command = Command::Quit;
          while (command != Command::Enter) {
            command = front.Input();
            front.render(a);
          }
        }
      }
      what_run = 'm';
      maps = get_maps();
    } else if (what_run == 'p') {
      using Clock = steady_clock;
      auto last = Clock::now();

      optional<Core> core_opt;
      try {
        core_opt.emplace(map_name);
      } catch (const exception &) { // битый/отсутствующий файл карты
        what_run = 'm';
        continue;
      }
      Core &core = *core_opt;

      while (core.running()) {
        auto now = Clock::now();
        double dt = duration<double>(now - last).count();
        last = now;
        auto command = front.Input();
        if (command.has_value())
          core.handle(*command);
        core.update(dt);
        front.render(core.get_state_pag(), core.get_state_ghost(),
                     core.get_map());
        this_thread::sleep_for(20ms);
      }
      command = front.Input();
      while (command != Command::Quit && command != Command::Enter) {
        front.render(core.get_state_pag().win, core.get_state_pag().score);
        command = front.Input();
      }
      what_run = 'm';
    }
  }
  return 0;
}
