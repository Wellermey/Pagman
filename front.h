#pragma once
#include "core.h"
#include <optional>

class Frontend {
public:
  Frontend();
  std::optional<Command> Input();
  void render(const State_pagman &state_pag, const State_ghost &State_ghost,
              const Map &map);
  void render(const State_menu &state_pag, string map_name);
  void render(bool win, int score);
  void render(Changeable stats);
  void render(Editor_state state);
  void render(int err);
  ~Frontend();
};
