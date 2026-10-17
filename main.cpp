
#include "core.h"
#include "front.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

int main() {

  Menu menu;
  Frontend front;

  while (menu.running()) {
    auto command = front.Input();
    if (command.has_value())
      menu.handle(*command);
    if (!menu.pagman())
      front.render(menu.get_state());
    else {
      using Clock = steady_clock;
      auto last = Clock::now();

      Core core(menu.name());

      while (core.running()) {
        auto now = Clock::now();
        double dt = duration<double>(now - last).count();
        last = now;
        auto command = front.Input();
        if (command.has_value())
          core.handle(*command);
        core.update(dt);
        front.render(core.get_state_pag(), core.get_state_ghost());
        this_thread::sleep_for(20ms);
      }
      menu = Menu();
    }
  }
  return 0;
}
