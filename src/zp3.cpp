#include "zp3.hpp"

int zp3_parse_metadata(const std::string &song_path,
                       song_t &song) {
  TagLib::FileRef meta(song_path.c_str());
  if (!meta.isNull() && meta.tag()) {
    TagLib::Tag *tag = meta.tag();
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

void zp3_print_artists(const zp3_t &zp3) {
  const auto keys = extract_keys<std::string, std::set<std::string>>(zp3.artists);
  for (const auto &key : keys) {
    printf("Artist: %s\n", key.c_str());
    for (const auto &album: zp3.artists.at(key)) {
      printf("Album: %s\n", album.c_str());
    }
    printf("\n");
  }
}

void zp3_print_albums(const zp3_t &zp3) {
  const auto keys = extract_keys<std::string, std::vector<song_t>>(zp3.albums);
  for (const auto &key : keys) {
    printf("Album: %s\n", key.c_str());
    for (const auto &song : zp3.albums.at(key)) {
      print_song(song);
      printf("\n");
    }
    printf("----------\n");
  }
}

void *zp3_player_thread(void *arg) {
  zp3_t *zp3 = (zp3_t *) arg;
  zp3->player_state = PLAY;

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
    while (zp3->player_state == PAUSE);

    // Stop?
    if (zp3->player_state == STOP) {
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
  zp3->song_path = "";
  zp3->player_state = STOP;
  zp3->player_is_dead = true;
  zp3->song_length = 0.0f;
  zp3->song_time = 0.0f;
}

int test_zp3_parse_metadata() {
  const auto song_path = "/data/music/Gorillaz_-_Demon_Days_(Remaster)_(2005)_FLAC/01 - Intro.mp3";
  song_t song;
  zp3_parse_metadata(song_path, song);

  return 0;
}

int test_zp3_player_thread() {
  // Setup
  mpg123_init();  // Do this only once!
  zp3_t zp3;
  zp3.song_path = "/data/music/great_success.mp3";

  pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);
  pthread_join(zp3.player_thread_id, NULL);

  return 0;
}

int test_zp3_load_library() {
  zp3_t zp3;
  zp3_load_library(zp3, "/data/music");
  zp3_print_artists(zp3);
  zp3_print_albums(zp3);

  return 0;
}

int main(int argc, char **argv) {
  // Setup
  mpg123_init();  // Do this only once!
  zp3_t zp3;

  // test_zp3_player_thread();
  // test_zp3_parse_metadata();
  // test_zp3_load_library();

  return 0;
}
