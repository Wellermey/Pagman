#pragma once
#include "core.h"
#include <optional>

class Frontend {
public:
  Frontend();
  std::optional<Command> Input();
  void render(const State_pagman &state_pag, const State_ghost &State_ghost);
  void render(const State_menu &state_pag);
  ~Frontend();
};
