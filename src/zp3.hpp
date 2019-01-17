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
#include "music.hpp"

#define KNRM "\x1B[1;0m"
#define KRED "\x1B[1;31m"
#define KGRN "\x1B[1;32m"
#define KYEL "\x1B[1;33m"
#define KBLU "\x1B[1;34m"
#define KMAG "\x1B[1;35m"
#define KCYN "\x1B[1;36m"
#define KWHT "\x1B[1;37m"

#define CHECK(COND) \
  if (COND != true) { \
    return -1; \
  }

#define RUN_TEST(TEST) \
  printf("%sTEST%s [%s] ", KBLU, KNRM, #TEST); \
  fflush(stdout); \
  if (TEST() == 0) { \
    printf("%sPASSED%s\n", KGRN, KNRM); \
  } else { \
    printf("%sFAILED%s\n", KRED, KNRM); \
  }

// ZP3 STATES
#define MENU 0
#define SONGS 1
#define ARTISTS 2
#define ALBUMS 3
#define PLAYER 4

#define PLAYER_PLAY 0
#define PLAYER_STOP 1
#define PLAYER_PAUSE 2

struct zp3_t {
  // Settings
  float min_volume = 0.0f;
  float max_volume = 1.0f;

  // State
  std::vector<int> history;
  float volume = 0.3f;
  std::string target_artist;
  std::string target_album;

  // Library
  songs_t songs;
  artists_t artists;
  albums_t albums;

  // Player
  pthread_t player_thread_id;
  std::string song_path;
  int player_state = PLAYER_STOP;
  bool player_is_dead = false;
  float song_length = 0.0f;
  float song_time = 0.0f;
};

#endif // ZP3_HPP
