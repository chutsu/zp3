#include "zp3.hpp"

void zp3_display_init(const zp3_t &zp3) {
#if ZP3_DISPLAY == DISPLAY_SDL
  ssd1351_128x128_spi_init(3, 4, 5);
#elif ZP3_DISPLAY == DISPLAY_HARDWARE
  // Raspberry mode (gpio24=RST, 0=CE, gpio23=D/C)
  ssd1351_128x128_spi_init(24, 0, 23);
#endif

  // Initialize display
  ssd1351_setMode(LCD_MODE_NORMAL);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen8();
}

void zp3_display_menu(zp3_t &zp3, const int index) {
  if (zp3.menu_items.size() > 0 && zp3.menu_set == false) {
    // Create and show menu
    const char *menu_list[10000];
    for (int i = 0; i < 10000; i++) {
      menu_list[i] = nullptr;
    }
    for (int i = 0; i < zp3.menu_items.size(); i++) {
      menu_list[i] = zp3.menu_items[i].c_str();
    }

    ssd1306_createMenu(&zp3.menu, menu_list, zp3.menu_items.size());
    if (index >= 0) {
      zp3.menu.selection = index;
    }
    ssd1306_showMenu8(&zp3.menu);
    zp3.menu_set = true;
  }

  ssd1306_clearScreen8();
  zp3.menu.selection = index;
  ssd1306_showMenu8(&zp3.menu);
  ssd1306_updateMenu8(&zp3.menu);
}

void zp3_display_song(zp3_t *zp3, const song_t &song) {
  ssd1306_clearScreen8();

  // Setup canvas
  const int track_scroll_counter = 0;
  const int screen_width = ssd1306_displayWidth();
  const int screen_height = ssd1306_displayHeight();
  uint8_t buffer[(screen_width * screen_height) / 8] = {0};
  NanoCanvas1_8 canvas(screen_width, screen_height, buffer);
  ssd1306_clearScreen8();
  canvas.clear();

  // Track name
  {
    const int x = 5 - track_scroll_counter;
    const int y = 20;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.title.c_str(), STYLE_NORMAL);
  }

  // Track artist
  {
    const int x = 5 - track_scroll_counter;
    const int y = 40;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.artist.c_str(), STYLE_NORMAL);
  }

  // Track album
  {
    const int x = 5 - track_scroll_counter;
    const int y = 55;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.album.c_str(), STYLE_NORMAL);
  }

  // Track progress
  {
    float track_progress = zp3->song_time / zp3->song_length;
    if (track_progress < 0 || zp3->song_length < 0.01) {
      track_progress = 0.0;
    } else if (track_progress > 1.0) {
      track_progress = 1.0;
    }
    // -- Progress outline
    const int top_left[2] = {10, 75};
    const int bottom_right[2] = {screen_width - 10, top_left[1] + 10};
    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    canvas.drawRect(top_left[0], top_left[1], bottom_right[0], bottom_right[1]);
    // -- Progress bar
    const int progress_bar_width = (screen_width - 10) - 10;
    const int progress_x = 10 + (progress_bar_width * track_progress);
    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    canvas.fillRect(top_left[0], top_left[1], progress_x, bottom_right[1]);
  }

  // Track status
  if (zp3->player_state == PLAYER_PAUSE) {
    // Pause box
    {
      const int width = 14;
      const int center[2] = {screen_width / 2, 105};
      const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
      const int p2[2] = {center[0] + width / 2, center[1] + width / 2};
      canvas.setColor(RGB_COLOR8(255, 255, 255));
      canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
    }

    // Pause split
    {
      const int height = 14;
      const int width = 4;
      const int center[2] = {screen_width / 2, 105};
      const int p1[2] = {center[0] - width / 2, center[1] - height / 2};
      const int p2[2] = {center[0] + width / 2, center[1] + height / 2};
      canvas.setColor(RGB_COLOR8(0, 0, 0));
      canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
    }

  } else if (zp3->player_state == PLAYER_PLAY) {
    const int width = 14;
    const int center[2] = {screen_width / 2, 105};
    const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
    const int p2[2] = {center[0] - width / 2, center[1] + width / 2};
    const int p3[2] = {center[0] + width / 2, center[1]};
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.drawLine(p1[0], p1[1], p2[0], p2[1]);
    canvas.drawLine(p2[0], p2[1], p3[0], p3[1]);
    canvas.drawLine(p3[0], p3[1], p1[0], p1[1]);

    // The following is pretty dumb but the canvas object provides
    // no api to draw a polygon and fill it with a color. This painful hack
    // will do for now. It essentially fills in the play symbol by filling
    // every single pixel, from left to right, top to bottom.
    for (int i = 0; i < 12; i++) {
      canvas.putPixel(p1[0] + 1, (center[1] - width / 2 + 1) + i);
    }
    for (int i = 0; i < 12; i++) {
      canvas.putPixel(p1[0] + 2, (center[1] - width / 2 + 2) + i);
    }
    for (int i = 0; i < 10; i++) {
      canvas.putPixel(p1[0] + 3, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 9; i++) {
      canvas.putPixel(p1[0] + 4, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 9; i++) {
      canvas.putPixel(p1[0] + 5, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 7; i++) {
      canvas.putPixel(p1[0] + 6, (center[1] - width / 2 + 4) + i);
    }
    for (int i = 0; i < 7; i++) {
      canvas.putPixel(p1[0] + 7, (center[1] - width / 2 + 4) + i);
    }
    for (int i = 0; i < 6; i++) {
      canvas.putPixel(p1[0] + 8, (center[1] - width / 2 + 5) + i);
    }
    for (int i = 0; i < 5; i++) {
      canvas.putPixel(p1[0] + 9, (center[1] - width / 2 + 5) + i);
    }
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 0);
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 1);
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 2);
    canvas.putPixel(p1[0] + 11, (center[1] - width / 2 + 7) + 0);
    canvas.putPixel(p1[0] + 11, (center[1] - width / 2 + 7) + 1);
    canvas.putPixel(p1[0] + 12, (center[1] - width / 2 + 7));

  } else if (zp3->player_state == PLAYER_STOP) {
    const int width = 14;
    const int center[2] = {screen_width / 2, 105};
    const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
    const int p2[2] = {center[0] + width / 2, center[1] + width / 2};
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
  }

  // Display canvas
  canvas.blt();
}

void zp3_display_clear(zp3_t &zp3) {
  zp3.menu_items.clear();
  zp3.menu_set = false;
  ssd1306_clearScreen8();
}

int zp3_parse_metadata(const std::string &song_path, song_t &song) {
  TagLib::FileRef meta(song_path.c_str());
  if (!meta.isNull() && meta.tag()) {
    TagLib::Tag *tag = meta.tag();
    song.file_path = song_path;
    song.title = tag->title().toCString();
    song.artist = tag->artist().toCString();
    song.album = tag->album().toCString();
    song.year = tag->year();
    song.track_number = tag->track();

    if (song.title == "" || song.artist == "") {
      return -1;
    }
  } else {
    return -1;
  }

  return 0;
}

int zp3_parse_metadata(const std::vector<std::string> &song_paths,
                       std::vector<song_t> &songs) {
  for (const auto &song_path: song_paths) {
    song_t song;
    if (zp3_parse_metadata(song_path, song) == 0) {
      songs.push_back(song);
    }
  }

  return 0;
}

int zp3_load_library(zp3_t &zp3, const std::string &path) {
  // Parse all songs
  std::vector<std::string> file_list;
  walkdir(path, file_list, "mp3");
  zp3.songs.clear();
  zp3_parse_metadata(file_list, zp3.songs);
  std::sort(zp3.songs.begin(), zp3.songs.end(), song_comparator);
  if (zp3.songs.size() == 0) {
    LOG_ERROR("No songs found at [%s]!", path.c_str());
    return -1;
  }

  // Get all artists
  zp3.artists.clear();
  for (const auto &song : zp3.songs) {
    zp3.artists[song.artist].insert(song.album);
  }

  // Get all albums
  zp3.albums.clear();
  for (const auto &song : zp3.songs) {
    zp3.albums[song.album].push_back(song);
  }

  return 0;
}

std::vector<song_t> zp3_filter_songs(const zp3_t &zp3) {
  std::vector<song_t> songs;

  if (zp3.target_artist == "" && zp3.target_album == "") {
    songs = zp3.songs;

  } else {
    for (const auto &song : zp3.songs) {
      bool add = true;
      if (zp3.target_artist != "" && zp3.target_artist != song.artist) {
        add = false;
      }
      if (zp3.target_album != "" && zp3.target_album != song.album) {
        add = false;
      }
      if (add) {
        songs.push_back(song);
      }
    }
  }

  return songs;
}

std::vector<std::string> zp3_filter_albums(const zp3_t &zp3) {
  auto keys = extract_keys<std::string, std::vector<song_t>>(zp3.albums);
  std::vector<std::string> albums;

  if (zp3.target_artist == "") {
    albums = keys;

  } else {
    for (const auto &album: keys) {
      const auto song = zp3.albums.at(album).at(0);
      if (zp3.target_artist == song.artist) {
        albums.push_back(album);
      }
    }
  }

  return albums;
}

void zp3_print_menu(zp3_t &zp3, const int index=-1) {
  int menu_index = 0;
  std::vector<std::string> menu_items = {"Songs", "Artists", "Albums"};

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  system("clear");
  for (const auto &entry : menu_items) {
    if (menu_index == index) {
      printf("%s", KGRN);
    }
    printf("%s\n", entry.c_str());
    if (menu_index == index) {
      printf("%s", KNRM);
    }
    menu_index++;
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL
  zp3.menu_items = menu_items;
  zp3_display_menu(zp3, index);
#endif
}

void zp3_print_songs(zp3_t &zp3, const int index) {
  int song_index = 0;

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  system("clear");
  for (const auto &song : zp3_filter_songs(zp3)) {
    if (song_index == index) {
      printf("%s", KGRN);
    }
    printf("%s - [%s]", song.artist.c_str(), song.title.c_str());
    if (song_index == index) {
      printf("%s", KNRM);
    }
    song_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL
  std::vector<std::string> songs;
  for (const auto &song : zp3_filter_songs(zp3)) {
    songs.emplace_back(song.artist + " - " + song.title);
  }
  zp3.menu_items = songs;
  zp3_display_menu(zp3, index);
#endif
}

std::string zp3_print_artists(zp3_t &zp3, const int index=-1) {
  int artist_index = 0;
  const auto keys = extract_keys<std::string, std::set<std::string>>(zp3.artists);

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  system("clear");
  for (const auto &key : keys) {
    if (artist_index == index) {
      printf("%s", KGRN);
    }
    printf("Artist: %s", key.c_str());
    if (artist_index == index) {
      printf("%s", KNRM);
    }
    artist_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL
  zp3.menu_items = keys;
  zp3_display_menu(zp3, index);
#endif

  return keys[index];
}

std::string zp3_print_albums(zp3_t &zp3, const int index=-1) {
  int album_index = 0;
  const auto albums = zp3_filter_albums(zp3);

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  system("clear");
  for (const auto &album : albums) {
    if (album_index == index) {
      printf("%s", KGRN);
    }
    printf("%s", album.c_str());
    if (album_index == index) {
      printf("%s", KNRM);
    }
    album_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL
  zp3.menu_items = albums;
  zp3_display_menu(zp3, index);
#endif

  return albums[index];
}

void *zp3_player_thread(void *arg) {
play_song:
  zp3_t *zp3 = (zp3_t *) arg;
  zp3->player_state = PLAYER_PLAY;

  // Initialize AO
  ao_initialize();
  int driver = ao_default_driver_id();

  // Initialize MPG123
  int err = 0;
  mpg123_handle *mh = mpg123_new(NULL, &err);
  size_t buffer_size = mpg123_outblock(mh);
  auto buffer = (unsigned char *) malloc(buffer_size * sizeof(unsigned char));

  // Open the file and get the decoding format
  const auto song = zp3->song_queue.at(zp3->song_index);
  mpg123_open(mh, song.file_path.c_str());

  // Get song format
  int channels = 0;
  int encoding = 0;
  long rate = 0;
  mpg123_getformat(mh, &rate, &channels, &encoding);

  // Set the output format and open the output device
  ao_sample_format format;
  format.bits = mpg123_encsize(encoding) * 8;  // 8 is number of bits
  format.rate = rate;
  format.channels = channels;
  format.byte_format = AO_FMT_NATIVE;
  format.matrix = 0;
  ao_device *dev = ao_open_live(driver, &format, NULL);

  // Play
  mpg123_volume(mh, zp3->volume);
  zp3->song_length = mpg123_framelength(mh) * mpg123_tpf(mh);
  const size_t frame_length = mpg123_framelength(mh);
  size_t done;

  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
    // Update song time
    zp3->song_time = (mpg123_tell(mh) / mpg123_spf(mh)) * mpg123_tpf(mh);
    zp3_display_song(zp3, song);

    // Pause?
    while (zp3->player_state == PLAYER_PAUSE);

    // Stop?
    if (zp3->player_state == PLAYER_STOP) {
      break;
    }

    // Keep playing
    ao_play(dev, (char *) buffer, done);

    // Set volume
    mpg123_volume(mh, zp3->volume);
  }

  // Clean up
  free(buffer);
  ao_close(dev);
  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();
  ao_shutdown();

  // Reset player
  // zp3->song_path = "";
  // zp3->player_state = PLAYER_STOP;
  zp3->player_is_dead = true;
  zp3->song_length = 0.0f;
  zp3->song_time = 0.0f;

  // Play next song if song queue is not finished
  if (zp3->player_state != PLAYER_STOP) {
    if ((zp3->song_index + 1) != zp3->song_queue.size()) {
      zp3->song_index++;
      goto play_song;
    }
  }
}

int zp3_init(zp3_t &zp3, const std::string &music_path) {
  mpg123_init();  // Do this only once!
  if (zp3_load_library(zp3, music_path)) {
    LOG_ERROR("Failed to load music library [%s]!", music_path.c_str());
  }

#if ZP3_DISPLAY == DISPLAY_SDL
  zp3_display_init(zp3);
#endif

  return 0;
}

int zp3_player_play(zp3_t &zp3) {
  if (zp3.player_state != PLAYER_STOP) {
    zp3.player_state = PLAYER_STOP;
    pthread_join(zp3.player_thread_id, NULL);
  }
  pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);

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

int test_zp3_filter_songs() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");

  // No filter
  auto songs = zp3_filter_songs(zp3);
  CHECK(songs.size() > 0);

  // Add artist filter
  zp3.target_artist = "Bob Dylan";
  songs = zp3_filter_songs(zp3);
  for (const auto &song : songs) {
    CHECK(song.artist == "Bob Dylan");
  }

  // Add artist filter
  zp3.target_artist = "Michael Jackson";
  zp3.target_album = "ALBUM3";
  songs = zp3_filter_songs(zp3);
  for (const auto &song : songs) {
    CHECK(song.artist == "Michael Jackson");
    CHECK(song.album == "ALBUM3");
  }

  return 0;
}

int test_zp3_filter_albums() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");

  // No filter
  auto albums = zp3_filter_albums(zp3);
  CHECK(albums.size() > 0);

  // Apply artist filter
  zp3.target_artist = "Michael Jackson";
  albums = zp3_filter_albums(zp3);
  CHECK(albums.size() == 2);
  CHECK(albums.at(0) == "ALBUM2");
  CHECK(albums.at(1) == "ALBUM3");

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

int test_zp3_parse_metadata() {
  const auto song_path = "./test_data/library/album1/1-apple.mp3";
  song_t song;
  zp3_parse_metadata(song_path, song);
  // print_song(song);

  CHECK(song.title == "Apple");
  CHECK(song.artist == "Bob Dylan");
  CHECK(song.album == "ALBUM1");
  CHECK(song.year == 2018);
  CHECK(song.track_number == 1);

  return 0;
}

int test_zp3_player_thread() {
  // Setup
  mpg123_init();  // Do this only once!
  zp3_t zp3;
  zp3.volume = 0.0;
  // zp3.song_queue = "./test_data/great_success.mp3";

  pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);
  pthread_join(zp3.player_thread_id, NULL);

  return 0;
}

int test_zp3_load_library() {
  zp3_t zp3;
  zp3_load_library(zp3, "./test_data/library");
  // zp3_print_artists(zp3);
  zp3.target_artist = "Bob Dylan";
  zp3_print_albums(zp3, 0);

  CHECK(zp3.artists.size() == 2);
  CHECK(zp3.albums.size() == 2);
  CHECK(zp3.songs.size() == 10);

  return 0;
}

int main(int argc, char **argv) {
  // Run tests
  // RUN_TEST(test_zp3_display_song);
  // RUN_TEST(test_zp3_filter_songs);
  // RUN_TEST(test_zp3_filter_albums);
  // // RUN_TEST(test_zp3_print_songs);
  // // RUN_TEST(test_zp3_print_artists);
  // // RUN_TEST(test_zp3_print_albums);
  // RUN_TEST(test_zp3_parse_metadata);
  // RUN_TEST(test_zp3_player_thread);
  // RUN_TEST(test_zp3_load_library);

  // Play
  zp3_t zp3;
  if (zp3_init(zp3, "/data/music") != 0) {
    FATAL("Failed to initialize ZP3!");
  }
  zp3_loop(zp3);

  return 0;
}
