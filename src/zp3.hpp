#ifndef ZP3_HPP
#define ZP3_HPP

#include <memory.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

#include <ao/ao.h>
#include <mpg123.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

#include "util.hpp"
#include "log.hpp"
#include "song.hpp"

// ZP3 STATES
#define MENU 0
#define SONGS 1
#define ARTISTS 2
#define ALBUMS 3

#define PLAY 0
#define STOP 1
#define PAUSE 2

struct zp3_t {
  // Settings
  float min_volume = 0.0f;
  float max_volume = 1.0f;

  // State
  int state = MENU;
  float volume = 0.3f;

  // Library
  std::vector<song_t> songs;
  std::map<std::string, std::set<std::string>> artists;
  std::map<std::string, std::vector<song_t>> albums;

  // Player
  pthread_t player_thread_id;
  std::string song_path;
  int player_state = STOP;
  bool player_is_dead = false;
  float song_length = 0.0f;
  float song_time = 0.0f;
};

#endif // ZP3_HPP
