
#include "core.h"
#include "front.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

vector<string> get_maps() {
  vector<string> maps;
  for (const auto &entry : filesystem::directory_iterator("maps")) {
    if (entry.is_regular_file() && entry.path().extension() == ".map") {
      maps.push_back(entry.path().filename().string());
    }
  }
  return maps;
}

void init_menu(int &menu_type, Menu &menu, bool &menu_run, bool &core_run,
               string &map_name, vector<string> maps) {
  if (menu_type == 'i')
    menu.init({
        {"Play", [&core_run] { core_run = true; }},
        {"Change map", [&menu_type] { menu_type = 'm'; }},
        {"Create map", [] { /* TODO */ }},
        {"Change stats", [] { /* TODO */ }},
        {"Quit", [&menu_run] { menu_run = false; }},
    });
  else if (menu_type == 'm') {
    vector<Item> v;
    for (int i = 0; i < (int)maps.size(); i++)
      v.push_back({maps[i], [i, &maps, &map_name] { map_name = maps[i]; }});
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
  bool menu_run = 1, core_run = 0;
  vector<string> maps = get_maps();

  while (menu_run) {
    if (menu_type) {
      init_menu(menu_type, menu, menu_run, core_run, map_name, maps);
    }
    auto command = front.Input();
    if (command.has_value())
      menu.handle(*command);
    if (!core_run)
      front.render(menu.get_state(), map_name);
    else {
      using Clock = steady_clock;
      auto last = Clock::now();

      Core core(map_name);

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
      core_run = 0;
    }
  }
  return 0;
}
