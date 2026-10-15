#pragma once
#include "core.h"
#include <optional>

class Frontend {
public:
  Frontend();
  std::optional<Command> Input();
  void render(const State_pagman &state);
  void render(const State_menu &state);
  ~Frontend();
};
