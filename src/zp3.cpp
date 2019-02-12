#include "zp3.hpp"

int zp3_init(zp3_t &zp3, const std::string &music_path) {
  if (music_load_library(zp3.music, music_path)) {
    LOG_ERROR("Failed to load music library [%s]!", music_path.c_str());
  }
  display_init();
  zp3.player.display = &zp3.display;

  return 0;
}

int zp3_menu_mode(zp3_t &zp3) {
  LOG_INFO("Menu mode");
  zp3.target_artist = "";
  zp3.target_album = "";

  int menu_index = zp3.main_menu_idx;
  display_show_menu(zp3.display, menu_index);
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
        display_clear(zp3.display);
        return menu_index + 1;
      case 'q':
        exit(0);
      default:
        continue;
    }

    display_show_menu(zp3.display, menu_index);
  }

  return -1;
}

int zp3_player_mode(zp3_t &zp3) {
  LOG_INFO("Player mode");
  player_play(zp3.player);

  // Listen for keyboard events
  while (true) {
    // display_song(&zp3, zp3.song_queue.at(zp3.song_index));

    switch (getch()) {
      case 'h': {
        player_stop(zp3.player);
        display_clear(zp3.display);

        const auto mode = zp3.history.back();
        zp3.history.pop_back();
        return mode;
      }
      case 'l':
        player_toggle_pause_play(zp3.player);
        break;
      case '+':
        player_volume_up(zp3.player);
        break;
      case '-':
        player_volume_down(zp3.player);
        break;
      default:
        continue;
    }
  }

  return -1;
}

int zp3_songs_mode(zp3_t &zp3) {
  LOG_INFO("Songs mode");
  int menu_idx = zp3.player.song_index;

  // Listen for keyboard events
  const auto music = zp3.music;
  const auto artist = zp3.target_artist;
  const auto album = zp3.target_album;
  const auto songs = music_filter_songs(music, artist, album);
  display_show_songs(zp3.display, songs, menu_idx);

  int max_entries = zp3.music.songs.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        display_clear(zp3.display);
        zp3.player.song_index = 0;
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
      case 'l': {
        const auto music = zp3.music;
        const auto artist = zp3.target_artist;
        const auto album = zp3.target_album;
        zp3.player.song_queue = music_filter_songs(music, artist, album);
        zp3.player.song_index = menu_idx;
        zp3.history.push_back(SONGS);
        return PLAYER;
      }
      default:
        continue;
    }

    display_show_songs(zp3.display, songs, menu_idx);
  }
}

int zp3_artists_mode(zp3_t &zp3) {
  LOG_INFO("Artists mode");
  int menu_idx = zp3.artists_menu_idx;
  std::string artist = display_show_artists(zp3.display,
                                            zp3.music.artists,
                                            menu_idx);

  int max_entries = zp3.music.artists.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        display_clear(zp3.display);
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
        display_clear(zp3.display);
        zp3.target_artist = artist;
        zp3.artists_menu_idx = menu_idx;
        zp3.history.push_back(ARTISTS);
        return ALBUMS;
      default:
        continue;
    }

    std::string artist = display_show_artists(zp3.display,
                                              zp3.music.artists,
                                              menu_idx);
  }
}

int zp3_albums_mode(zp3_t &zp3) {
  LOG_INFO("Albums mode");
  int menu_idx = zp3.albums_menu_idx;

  // Filter and show albums
  std::vector<std::string> album_names;
  for (const auto &album : music_filter_albums(zp3.music, zp3.target_artist)) {
    album_names.emplace_back(album);
  }
  std::string album = display_show_albums(zp3.display, album_names, menu_idx);

  // Event handler
  int max_entries = zp3.music.albums.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h': {
        display_clear(zp3.display);
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
        display_clear(zp3.display);
        zp3.target_album = album;
        zp3.albums_menu_idx = menu_idx;
        zp3.history.push_back(ALBUMS);
        return SONGS;
      default:
        continue;
    }
    album = display_show_albums(zp3.display, album_names, menu_idx);
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
      default: FATAL("Programming Error!"); break;
    }
  }
  return 0;
}
