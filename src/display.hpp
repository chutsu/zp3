#ifndef ZP3_DISPLAY_HPP
#define ZP3_DISPLAY_HPP

#include "music.hpp"

#include <string>
#include <vector>

#include <ssd1306.h>
#include <nano_engine.h>

#define DISPLAY_CONSOLE 0
#define DISPLAY_SDL 1
#define DISPLAY_HARDWARE 2
#ifndef ZP3_DISPLAY
  #define ZP3_DISPLAY DISPLAY_SDL
#endif

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
void display_show_menu(display_t &display, const int index);
void display_show_songs(display_t &display,
                        const songs_t &songs,
                        const int index);
std::string display_show_artists(display_t &display,
                                 const artists_t &artists,
                                 const int index);
std::string display_show_albums(display_t &display,
                                const std::vector<std::string> &albums,
                                const int index);

#endif // ZP3_DISPLAY_HPP
