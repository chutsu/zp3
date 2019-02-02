#ifndef ZP3_HPP
#define ZP3_HPP

#include "util.hpp"
#include "log.hpp"
#include "music.hpp"
#include "player.hpp"
#include "display.hpp"

// ZP3 STATES
#define MENU 0
#define SONGS 1
#define ARTISTS 2
#define ALBUMS 3
#define PLAYER 4

struct zp3_t {
  // State
  std::vector<int> history;
  std::string target_artist;
  std::string target_album;
  int main_menu_idx = 0;
  int artists_menu_idx = 0;
  int albums_menu_idx = 0;

  music_t music;
  display_t display;
  player_t player;
};

int zp3_init(zp3_t &zp3, const std::string &music_path);
int zp3_menu_mode(zp3_t &zp3);
int zp3_player_mode(zp3_t &zp3);
int zp3_songs_mode(zp3_t &zp3);
int zp3_artists_mode(zp3_t &zp3);
int zp3_albums_mode(zp3_t &zp3);
int zp3_loop(zp3_t &zp3);

#endif // ZP3_HPP
