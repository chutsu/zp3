#include "test.hpp"
#include "display.hpp"

int test_display_init() {
  display_init();
  return 0;
}

int test_display_menu() {
  display_t display;
  display.menu_items = {"Songs", "Artists", "Albums"};

  display_menu(display, 0);
  sleep(2);

  return 0;
}

int test_display_song() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);
  song_t song = music.songs.at(0);

  // Test pause
  {
    display_t display;
    display_init();
    display_song(display, PLAYER_PAUSE, song, 0.01, 1.0);
    sleep(1);
  }

  // Test stop
  {
    display_t display;
    display_init();
    display_song(display, PLAYER_STOP, song, 0.01, 1.0);
    sleep(1);
  }

  // Test play
  {
    display_t display;
    display_init();
    display_song(display, PLAYER_PLAY, song, 0.01, 1.0);
    sleep(1);
  }

  return 0;
}

int test_display_show_songs() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  display_t display;
  display_show_songs(display, music.songs, 0);
  sleep(2);

  return 0;
}

int test_display_show_artists() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  display_t display;
  display_show_artists(display, music.artists, 0);
  sleep(2);

  return 0;
}

int test_display_show_albums() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  std::vector<std::string> album_names;
  for (const auto &album : music.albums) {
    album_names.emplace_back(album.first);
  }

  display_t display;
  display_show_albums(display, album_names, 0);
  sleep(2);

  return 0;
}

int main(int argc, char **argv) {
  RUN_TEST(test_display_init);
  RUN_TEST(test_display_menu);
  RUN_TEST(test_display_song);
  RUN_TEST(test_display_show_songs);
  RUN_TEST(test_display_show_artists);
  RUN_TEST(test_display_show_albums);

  return 0;
}
