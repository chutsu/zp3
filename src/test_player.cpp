#include "test.hpp"
#include "player.hpp"

int test_player_init() {
  player_init();  // Do this only once!
  return 0;
}

int test_player_thread() {
  // Load a song
  song_t song;
  song_parse_metadata(song, TEST_SONG);

  // Prepare player
  player_t player;
  player.volume = 0.0;
  player.song_queue.push_back(song);

  // Execute player thread
  std::thread thread{player_thread, &player};
  thread.join();

  return 0;
}

int test_player_play() {
  // Load a song
  song_t song;
  song_parse_metadata(song, TEST_SONG);

  // Prepare player
  display_t display;
  player_t player;
  player.volume = 0.0;
  player.display = &display;
  player.song_queue.push_back(song);

  // Play
  player_play(player);
  player.thread.join();

  return 0;
}

int test_player_stop() {
  player_t player;

  player.player_state = PLAYER_PLAY;
  player_stop(player);
  CHECK(player.player_state == PLAYER_STOP);

  return 0;
}

int test_player_toggle_pause_play() {
  player_t player;

  player.player_state = PLAYER_PLAY;
  player_toggle_pause_play(player);
  CHECK(player.player_state == PLAYER_PAUSE);

  player_toggle_pause_play(player);
  CHECK(player.player_state == PLAYER_PLAY);

  return 0;
}

int test_player_volume_up() {
  player_t player;
  player.volume = 0.0;

  player_volume_up(player);
  CHECK(player.volume > 0.0);

  return 0;
}

int test_player_volume_down() {
  player_t player;
  player.volume = 1.0;

  player_volume_down(player);
  CHECK(player.volume < 1.0);

  return 0;
}

int main(int argc, char **argv) {
  RUN_TEST(test_player_init);
  RUN_TEST(test_player_thread);
  RUN_TEST(test_player_play);
  RUN_TEST(test_player_stop);
  RUN_TEST(test_player_toggle_pause_play);
  RUN_TEST(test_player_volume_up);
  RUN_TEST(test_player_volume_down);

  return 0;
}
