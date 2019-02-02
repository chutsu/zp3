#ifndef ZP3_HPP
#define ZP3_HPP

#include "util.hpp"
#include "log.hpp"
#include "music.hpp"
#include "font_verdana.hpp"
#include "font_freemono.hpp"

// ZP3 STATES
#define MENU 0
#define SONGS 1
#define ARTISTS 2
#define ALBUMS 3
#define PLAYER 4

struct zp3_t {
  // Settings
  float min_volume = 0.0f;
  float max_volume = 1.0f;
  float volume_delta = 0.05f;

  // State
  std::vector<int> history;
  std::string target_artist;
  std::string target_album;
  int main_menu_idx = 0;
  int artists_menu_idx = 0;
  int albums_menu_idx = 0;
};

#endif // ZP3_HPP
