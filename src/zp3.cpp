#include "zp3.hpp"

int zp3_init(zp3_t &zp3, const std::string &music_path) {
  if (zp3_load_library(zp3, music_path)) {
    LOG_ERROR("Failed to load music library [%s]!", music_path.c_str());
  }

#if ZP3_DISPLAY == DISPLAY_SDL
  zp3_display_init(zp3);
#endif

  return 0;
}

int zp3_menu_mode(zp3_t &zp3) {
  LOG_INFO("Menu mode");
  zp3.target_artist = "";
  zp3.target_album = "";

  int menu_index = zp3.main_menu_idx;
  zp3_print_menu(zp3, menu_index);
  while (true) {
    switch (getch()) {
      case 'j':
        menu_index++;
        menu_index = (menu_index > 2) ? 2 : menu_index;
        break;
      case 'k':
        menu_index--;
        menu_index = (menu_index < 0) ? 0 : menu_index;
        break;
      case 'l':
        zp3.main_menu_idx = menu_index;
        zp3.history.push_back(MENU);
        zp3_display_clear(zp3);
        return menu_index + 1;
      case 'q':
        exit(0);
      default:
        continue;
    }

    zp3_print_menu(zp3, menu_index);
  }

  return -1;
}

int zp3_player_mode(zp3_t &zp3) {
  LOG_INFO("Player mode");
  zp3_player_play(zp3);

  // Listen for keyboard events
  while (true) {
    zp3_display_song(&zp3, zp3.song_queue.at(zp3.song_index));

    switch (getch()) {
      case 'h': {
        zp3.player_state = PLAYER_STOP;
        sleep(1);
        zp3_display_clear(zp3);

        const auto mode = zp3.history.back();
        zp3.history.pop_back();
        return mode;
      }
      case 'l':
        if (zp3.player_state == PLAYER_PLAY) {
          zp3.player_state = PLAYER_PAUSE;
        } else if (zp3.player_state == PLAYER_PAUSE) {
          zp3.player_state = PLAYER_PLAY;
        }
        break;
      case '+':
        zp3.volume += zp3.volume_delta;
        zp3.volume = (zp3.volume > 1.0) ? 1.0 : zp3.volume;
        break;
      case '-':
        zp3.volume -= zp3.volume_delta;
        zp3.volume = (zp3.volume < 0.0) ? 0.0 : zp3.volume;
        break;
      default:
        continue;
    }
  }

  return -1;
}

int zp3_songs_mode(zp3_t &zp3) {
  LOG_INFO("Songs mode");
  int menu_idx = zp3.song_index;

  // Listen for keyboard events
  zp3_print_songs(zp3, menu_idx);
  int max_entries = zp3.songs.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        zp3_display_clear(zp3);
        zp3.song_index = 0;
        const auto mode = zp3.history.back();
        zp3.history.pop_back();
        return mode;
      }
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx > max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'l':
        zp3.song_queue = zp3_filter_songs(zp3);
        zp3.song_index = menu_idx;
        zp3.history.push_back(SONGS);
        // zp3_player_play(zp3);
        return PLAYER;
      default:
        continue;
    }
    zp3_print_songs(zp3, menu_idx);
  }
}

int zp3_artists_mode(zp3_t &zp3) {
  LOG_INFO("Artists mode");
  int menu_idx = zp3.artists_menu_idx;
  std::string artist = zp3_print_artists(zp3, menu_idx);

  int max_entries = zp3.artists.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        zp3_display_clear(zp3);
        const auto mode = zp3.history.back();
        zp3.history.pop_back();
        zp3.artists_menu_idx = 0;
        return mode;
      }
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx >= max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'l':
        zp3_display_clear(zp3);
        zp3.target_artist = artist;
        zp3.artists_menu_idx = menu_idx;
        zp3.history.push_back(ARTISTS);
        return ALBUMS;
      default:
        continue;
    }
    artist = zp3_print_artists(zp3, menu_idx);
  }
}

int zp3_albums_mode(zp3_t &zp3) {
  LOG_INFO("Albums mode");
  int menu_idx = zp3.albums_menu_idx;
  std::string album = zp3_print_albums(zp3, menu_idx);

  int max_entries = zp3.albums.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        zp3_display_clear(zp3);
        const auto mode = zp3.history.back();
        zp3.history.pop_back();
        zp3.albums_menu_idx = 0;
        return mode;
      }
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx >= max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'l':
        zp3_display_clear(zp3);
        zp3.target_album = album;
        zp3.albums_menu_idx = menu_idx;
        zp3.history.push_back(ALBUMS);
        return SONGS;
      default:
        continue;
    }
    album = zp3_print_albums(zp3, menu_idx);
  }
}

int zp3_loop(zp3_t &zp3) {
  int retval = MENU;
  while (true) {
    switch (retval) {
      case MENU: retval = zp3_menu_mode(zp3); break;
      case SONGS: retval = zp3_songs_mode(zp3); break;
      case ARTISTS: retval = zp3_artists_mode(zp3); break;
      case ALBUMS: retval = zp3_albums_mode(zp3); break;
      case PLAYER: retval = zp3_player_mode(zp3); break;
    }
  }
  return 0;
}

int test_zp3_display_song() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");

  auto songs = zp3_filter_songs(zp3);
  zp3_display_init(zp3);

  zp3.player_state = PLAYER_PAUSE;
  zp3_display_song(&zp3, songs[0]);
  sleep(2);

  zp3.player_state = PLAYER_STOP;
  zp3_display_song(&zp3, songs[0]);
  sleep(2);

  zp3.player_state = PLAYER_PLAY;
  zp3_display_song(&zp3, songs[0]);
  sleep(2);

  return 0;
}


int test_zp3_print_songs() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");

  // No filter
  zp3_print_songs(zp3, 0);

  // Apply artist filter
  zp3.target_artist = "Michael Jackson";
  zp3.target_album = "ALBUM3";
  zp3_print_songs(zp3, 0);

  return 0;
}

int test_zp3_print_artists() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");
  zp3_print_artists(zp3, 0);

  return 0;
}

int test_zp3_print_albums() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");

  // No filter
  zp3_print_albums(zp3, 0);

  // Apply artist filter
  zp3.target_artist = "Michael Jackson";
  zp3_print_albums(zp3, 0);

  return 0;
}


int main(int argc, char **argv) {
  // Run tests
  // RUN_TEST(test_zp3_display_song);
  // // RUN_TEST(test_zp3_print_songs);
  // // RUN_TEST(test_zp3_print_artists);
  // // RUN_TEST(test_zp3_print_albums);
  // RUN_TEST(test_zp3_player_thread);

  // Play
  zp3_t zp3;
  if (zp3_init(zp3, "/data/music") != 0) {
    FATAL("Failed to initialize ZP3!");
  }
  zp3_loop(zp3);

  return 0;
}
