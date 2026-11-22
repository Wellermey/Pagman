#include "core.h"
#include "front.h"
#include "raylib.h"

#define BASECOLOR GetColor(0x1E1E2EFF)
#define TEXTCOLOR GetColor(0xCDD6F4FF)
#define REDCOLOR GetColor(0xF38BA8FF)
#define MANTLECOLOR GetColor(0x181825FF)
#define GREENCOLOR GetColor(0xa6e3a1FF)

Frontend::Frontend() {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Pagman");
}
std::optional<Command> Frontend::Input() {
  int ch = GetKeyPressed();
  switch (ch) {
  case 'q':
  case 'Q':
    return Command::Quit;
  case KEY_UP:
    return Command::Up;
  case KEY_DOWN:
    return Command::Down;
  case KEY_LEFT:
    return Command::Left;
  case KEY_RIGHT:
    return Command::Right;
  case KEY_ENTER:
    return Command::Enter;
  default:
    return std::nullopt;
  }
}
void Frontend::render(const State_pagman &state_pag,
                      const State_ghost &state_ghost, const Map &map) {}
void Frontend::render(const State_menu &state, string map_name) {
  constexpr int TITLE_SIZE = 60;
  constexpr int FONT = 25;
  constexpr int BTN_H = 44;
  constexpr int GAP = 14;
  constexpr int PAD_X = 40;
  constexpr int BORDER = 3;
  constexpr float ROUND = 0.35f;
  const char *title = "PAGMAN";

  int sw = GetScreenWidth();
  int sh = GetScreenHeight();
  bool no_maps = 1;
  int count = 0, maxw = 0;

  for (const auto &s : state.menu) {
    if (s == map_name)
      no_maps = 0;
    count++;
    maxw = std::max(maxw, MeasureText(s.c_str(), FONT));
  }

  int btn_w = std::max(260, maxw + 2 * PAD_X);
  int buttons_h = count > 0 ? count * BTN_H + (count - 1) * GAP : 0;
  int title_block = TITLE_SIZE + 40;
  int top = (sh - (title_block + buttons_h)) / 2;
  int btn_y0 = top + title_block;

  BeginDrawing();
  ClearBackground(BASECOLOR);
  if (no_maps)
    DrawText(title, (sw - MeasureText(title, TITLE_SIZE)) / 2, top, TITLE_SIZE,
             TEXTCOLOR);

  int row = 0;
  for (int i = 0; i < (int)state.menu.size(); i++) {
    bool selected = (i == state.selected);
    bool map = (map_name == state.menu[i]);
    const char *text = state.menu[i].c_str();

    Rectangle r = {(sw - btn_w) / 2.0f, (float)(btn_y0 + row * (BTN_H + GAP)),
                   (float)btn_w, (float)BTN_H};

    if (selected) {
      Rectangle o = {r.x - BORDER, r.y - BORDER, r.width + 2 * BORDER,
                     r.height + 2 * BORDER};
      DrawRectangleRounded(o, ROUND, 8, TEXTCOLOR);
    }
    DrawRectangleRounded(r, ROUND, 8, selected ? REDCOLOR : MANTLECOLOR);

    int ty = (int)r.y + (BTN_H - FONT) / 2;
    DrawText(text, (int)(r.x + (r.width - MeasureText(text, FONT)) / 2), ty,
             FONT,
             map        ? GREEN
             : selected ? BASECOLOR
                        : TEXTCOLOR);

    if (selected) {
      DrawText(">", (int)r.x + 15, ty, FONT, BASECOLOR);
      DrawText("<", (int)(r.x + r.width) - 15 - MeasureText("<", FONT), ty,
               FONT, BASECOLOR);
    }
    row++;
  }

  EndDrawing();
}

void Frontend::render(const Changeable stats) {}

void Frontend::render(const Editor_state state) {}

void Frontend::render(int err) {}

void Frontend::render(bool win, int score) {}

Frontend::~Frontend() {}
