
#include "core.h"
#include "front.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

int main() {

  using Clock = steady_clock;
  auto last = Clock::now();

  Core core("map01.txt");
  Frontend front;

  while (core.running()) {
    auto now = Clock::now();
    double dt = duration<double>(now - last).count();
    last = now;
    auto command = front.Input();
    if (command.has_value())
      core.handle(*command);
    core.update(dt);
    front.render(core.get_state());
    this_thread::sleep_for(20ms);
  }
  return 0;
}
