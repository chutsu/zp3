#ifndef ZP3_PLAYER_HPP
#define ZP3_PLAYER_HPP

#include <pthread.h>
#include <vector>

#include <ao/ao.h>
#include <mpg123.h>

#include "music.hpp"
#include "display.hpp"

#define PLAYER_PLAY 0
#define PLAYER_STOP 1
#define PLAYER_PAUSE 2


struct player_t {
  std::vector<song_t> song_queue;
  int song_index = 0;
  int player_state = PLAYER_STOP;
  bool player_is_dead = false;
  float song_length = 0.0f;
  float song_time = 0.0f;

  float volume = 0.3f;
};

void player_init();
void *player_thread(void *arg);
int player_play(player_t &player, pthread_t &thread_id);

#endif // ZP3_PLAYER_HPP
