#ifndef ZP3_DISPLAY_HPP
#define ZP3_DISPLAY_HPP

#include "music.hpp"

#include <string>
#include <vector>

#include <ssd1306.h>
#include <nano_engine.h>

// #include "font_verdana.hpp"
// #include "font_freemono.hpp"

#define DISPLAY_CONSOLE 0
#define DISPLAY_SDL 1
#define DISPLAY_HARDWARE 2
#ifndef ZP3_DISPLAY
  #define ZP3_DISPLAY DISPLAY_SDL
#endif

#define PLAYER_PLAY 0
#define PLAYER_STOP 1
#define PLAYER_PAUSE 2

struct menu_t {
  bool configured = false;
  const size_t max_chars = 21;
  const size_t max_entries = 12;
  std::vector<std::string> entries;
};

struct display_t {
  menu_t menu;
  const int width = ssd1306_displayWidth();
  const int height = ssd1306_displayHeight();
};

void menu_init(menu_t &menu, const std::vector<std::string> entries);
void menu_clear(menu_t &menu);
std::vector<std::string> menu_get_page(menu_t &menu, const int index);

void display_init();
void display_menu(display_t &display, const int selection_idx, const int scroll_idx=0);
void display_song(display_t &display,
                  const int player_state,
                  const song_t &song,
                  const float song_time,
                  const float song_length);
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
