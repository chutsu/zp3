#include "zp3.hpp"

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

void zp3_print_menu(const zp3_t &zp3, const int index=-1) {
  int menu_index = 0;
  std::vector<std::string> menu_items = {"Songs", "Artists", "Albums"};

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
}

void zp3_print_songs(const zp3_t &zp3, const int index) {
  int song_index = 0;

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
}

std::string zp3_print_artists(const zp3_t &zp3, const int index=-1) {
  int artist_index = 0;
  const auto keys = extract_keys<std::string, std::set<std::string>>(zp3.artists);

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

  return keys[index];
}

std::string zp3_print_albums(const zp3_t &zp3, const int index=-1) {
  int album_index = 0;
  const auto albums = zp3_filter_albums(zp3);

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

  return albums[index];
}

void *zp3_player_thread(void *arg) {
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
  mpg123_open(mh, zp3->song_path.c_str());

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
  zp3->player_state = PLAYER_STOP;
  zp3->player_is_dead = true;
  zp3->song_length = 0.0f;
  zp3->song_time = 0.0f;
}

int zp3_init(zp3_t &zp3, const std::string &music_path) {
  mpg123_init();  // Do this only once!
  if (zp3_load_library(zp3, music_path)) {
    LOG_ERROR("Failed to load music library [%s]!", music_path.c_str());
  }
  return 0;
}

int zp3_play(zp3_t &zp3) {
  if (zp3.player_state != PLAYER_STOP) {
    zp3.player_state = PLAYER_STOP;
    pthread_join(zp3.player_thread_id, NULL);
  }
  pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);

  return 0;
}

int zp3_songs_mode(zp3_t &zp3) {
  zp3_print_songs(zp3, 0);

  int menu_idx = 0;
  int max_entries = zp3.songs.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h':
        return -1;
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx > max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'p':
        zp3.song_path = zp3.songs[menu_idx].file_path;
        zp3_play(zp3);
        break;
      case '+':
        zp3.volume += 0.05;
        zp3.volume = (zp3.volume > 1.0) ? 1.0 : zp3.volume;
        break;
      case '-':
        zp3.volume -= 0.05;
        zp3.volume = (zp3.volume < 0.0) ? 0.0 : zp3.volume;
        break;
      default:
        continue;
    }

    system("clear");
    zp3_print_songs(zp3, menu_idx);
  }
}

int zp3_artists_mode(zp3_t &zp3) {
  std::string artist = zp3_print_artists(zp3, 0);

  int menu_idx = 0;
  int max_entries = zp3.artists.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h':
        return -1;
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx >= max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'l':
        zp3.target_artist = artist;
        zp3.prev_mode = MODE_ARTISTS;
        return 2;
      default:
        continue;
    }

    system("clear");
    artist = zp3_print_artists(zp3, menu_idx);
  }
}

int zp3_albums_mode(zp3_t &zp3) {
  std::string album = zp3_print_albums(zp3, 0);

  int menu_idx = 0;
  int max_entries = zp3.albums.size() - 1;
  while (true) {
    switch (getch()) {
      case 'h':
        return -1;
      case 'j':
        menu_idx++;
        menu_idx = (menu_idx >= max_entries) ? max_entries : menu_idx;
        break;
      case 'k':
        menu_idx--;
        menu_idx = (menu_idx < 0) ? 0 : menu_idx;
        break;
      case 'l':
        zp3.target_album = album;
        zp3.prev_mode = MODE_ALBUMS;
        return 0;
      default:
        continue;
    }

    system("clear");
    album = zp3_print_albums(zp3, menu_idx);
  }
}

int zp3_menu_mode(zp3_t &zp3) {
  zp3.target_artist = "";
  zp3.target_album = "";
  zp3_print_menu(zp3, 0);

  int menu_index = 0;
  std::vector<std::string> menu_items = {"Songs", "Artists", "Albums"};
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
        return menu_index;
      default:
        continue;
    }

    zp3_print_menu(zp3, menu_index);
  }

  return -1;
}

int zp3_loop(zp3_t &zp3) {
  int retval = -1;
  while (true) {
    switch (retval) {
      case -1: retval = zp3_menu_mode(zp3); break;
      case 0: retval = zp3_songs_mode(zp3); break;
      case 1: retval = zp3_artists_mode(zp3); break;
      case 2: retval = zp3_albums_mode(zp3); break;
    }
  }
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
  zp3.song_path = "./test_data/great_success.mp3";

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

  // CHECK(zp3.artists.size() == 2);
  // CHECK(zp3.albums.size() == 2);
  // CHECK(zp3.songs.size() == 10);

  return 0;
}

int main(int argc, char **argv) {
  // Run tests
  // RUN_TEST(test_zp3_filter_songs);
  // RUN_TEST(test_zp3_filter_albums);
  // RUN_TEST(test_zp3_print_songs);
  // RUN_TEST(test_zp3_print_artists);
  // RUN_TEST(test_zp3_print_albums);
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
