#include "test.hpp"
#include "player.hpp"

int test_player_init() {
  player_init();  // Do this only once!
  return 0;
}

int test_player_thread() {
  // Load a song
  song_t song;
  song_parse_metadata(song, "./test_data/great_success.mp3");

  // Prepare player
  player_t player;
  player.volume = 0.0;
  player.song_queue.push_back(song);

  // Execute player thread
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, player_thread, &player);
  pthread_join(thread_id, NULL);

  return 0;
}

int test_player_play() {
  // Load a song
  song_t song;
  song_parse_metadata(song, "./test_data/great_success.mp3");

  // Prepare player
  player_t player;
  player.volume = 0.0;
  player.song_queue.push_back(song);

  // Play
  pthread_t thread_id;
  player_play(player, thread_id);
  pthread_join(thread_id, NULL);

  return 0;
}

int main(int argc, char **argv) {
  RUN_TEST(test_player_init);
  RUN_TEST(test_player_thread);
  RUN_TEST(test_player_play);

  return 0;
}
