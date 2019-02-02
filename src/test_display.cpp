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

int test_display_show_songs() {
  music_t music;
  music_load_library(music, "./test_data/library");

  display_t display;
  display_show_songs(display, music.songs, 0);
  sleep(2);

  return 0;
}

int test_display_show_artists() {
  music_t music;
  music_load_library(music, "./test_data/library");

  display_t display;
  display_show_artists(display, music.artists, 0);
  sleep(2);

  return 0;
}

int test_display_show_albums() {
  music_t music;
  music_load_library(music, "./test_data/library");

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
  RUN_TEST(test_display_show_songs);
  RUN_TEST(test_display_show_artists);
  RUN_TEST(test_display_show_albums);

  return 0;
}
