#pragma once
#include "core.h"
#include <optional>

class Frontend {
public:
  Frontend();
  std::optional<Command> Input();
  void render(const State &state);
  ~Frontend();
};
