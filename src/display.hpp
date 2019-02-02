#ifndef ZP3_DISPLAY_HPP
#define ZP3_DISPLAY_HPP

#include "music.hpp"

#include <string>
#include <vector>

#include <ssd1306.h>
#include <nano_engine.h>

#define PLAYER_PLAY 0
#define PLAYER_STOP 1
#define PLAYER_PAUSE 2

struct display_t {
  SAppMenu menu;
  bool menu_set = false;
  std::vector<std::string> menu_items;
};

void display_init();
void display_menu(display_t &display, const int index);
void display_clear(display_t &display);

#endif // ZP3_DISPLAY_HPP
