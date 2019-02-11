#include "test.hpp"
#include "display.hpp"

int test_menu_init() {
  std::vector<std::string> entries = {"A", "B", "C"};
  menu_t menu;
  menu_init(menu, entries);

  CHECK(menu.configured);
  CHECK(menu.entries.size() == 3);

  return 0;
}

int test_menu_clear() {
  std::vector<std::string> entries;
  for (int i = 0; i < 30; i++) {
    entries.emplace_back(std::to_string(i));
  }

  menu_t menu;
  menu_init(menu, entries);
  CHECK(menu.configured);
  CHECK(menu.entries.size() == 30);

  menu_clear(menu);
  CHECK(menu.configured == false);
  CHECK(menu.entries.size() == 0);

  return 0;
}

int test_menu_get() {
  // Create test data
  std::vector<std::string> entries;
  for (int i = 0; i < 30; i++) {
    entries.emplace_back(std::to_string(i));
  }

  // Initialize menu
  menu_t menu;
  menu_init(menu, entries);

  // Get page 1
  auto retval = menu_get_page(menu, 5);
  CHECK(retval.size() == 12);

  // Get page 2
  retval = menu_get_page(menu, 13);
  CHECK(retval.size() == 12);

  // Get page 3
  retval = menu_get_page(menu, 29);
  CHECK(retval.size() == 6);

  return 0;
}

int test_display_init() {
  display_init();
  return 0;
}

int test_display_menu() {
  display_t display;

  printf("Select Songs\n");
  menu_init(display.menu, {"Songs", "Artists", "Albums"});
  display_menu(display, 0);
  sleep(1);

  printf("Select Artists\n");
  menu_init(display.menu, {"Songs", "Artists", "Albums"});
  display_menu(display, 1);
  sleep(1);

  printf("Select Albums\n");
  menu_init(display.menu, {"Songs", "Artists", "Albums"});
  display_menu(display, 2);
  sleep(1);

  printf("Select under lower bounds\n");
  menu_init(display.menu, {"Songs", "Artists", "Albums"});
  display_menu(display, -1);
  sleep(1);

  printf("Select above upper bounds\n");
  menu_init(display.menu, {"Songs", "Artists", "Albums"});
  display_menu(display, 3);
  sleep(1);

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

int test_display_show_menu() {
  display_t display;
  display_show_menu(display, 0);
  sleep(1);
  return 0;
}

int test_display_show_songs() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  // Show beginning of all songs
  {
    display_t display;
    display_show_songs(display, music.songs, 0);
    sleep(2);
  }

  // Show end of all songs
  {
    display_t display;
    display_show_songs(display, music.songs, 14);
    sleep(2);
  }

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

int test_display_scroll_text() {
  display_t display;
  menu_init(display.menu,
            {"123456791011121314151617181920",
             "123456791011121314151617181920",
             "123456791011121314151617181920"});

  int scroll_idx = 0;
  for (int i = 0; i < 6; i++) {
    scroll_idx = i;
    display_menu(display, 0, scroll_idx);
    usleep(0.5 * 1e6);
  }
  sleep(1);

  for (int i = 0; i < 20; i++) {
    scroll_idx = i;
    display_menu(display, 2, scroll_idx);
    usleep(0.5 * 1e6);
  }
  sleep(1);

  return 0;
}

int main(int argc, char **argv) {
  RUN_TEST(test_menu_init);
  RUN_TEST(test_menu_clear);
  RUN_TEST(test_menu_get);

  RUN_TEST(test_display_init);
  // RUN_TEST(test_display_menu);
  // RUN_TEST(test_display_song);
  // RUN_TEST(test_display_show_menu);
  // RUN_TEST(test_display_show_songs);
  // RUN_TEST(test_display_show_artists);
  // RUN_TEST(test_display_show_albums);
  RUN_TEST(test_display_scroll_text);

  return 0;
}
