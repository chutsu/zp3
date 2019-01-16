#include "zp3.hpp"

// ZP3 STATES
#define MENU 0
#define SONGS 1
#define ARTISTS 2
#define ALBUMS 3

#define PLAY 0
#define STOP 1
#define PAUSE 2

struct zp3_t {
  float min_volume = 0.0f;
  float max_volume = 1.0f;

  int state = MENU;
  float volume = 0.3f;

  pthread_t player_thread_id;
  std::string song_path;
  int player_state = STOP;
  bool player_is_dead = false;
  float song_length = 0.0f;
  float song_time = 0.0f;
};

struct song_t {
  std::string file_path;
  std::string title;
  std::string artist;
  std::string album;
  std::string year;
};

void walkdir(const std::string &path,
             std::vector<std::string> &file_list,
             const std::string &target_ext="*") {
  DIR *dir = opendir(path.c_str());
  if (dir == NULL) {
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string value(entry->d_name);
    if (value == "." || value == "..") {
      continue;
    }

    const auto ext_len = target_ext.length();
    const auto file_ext = value.substr(value.length() - ext_len, ext_len);
    if (entry->d_type == DT_DIR) {
      walkdir(path + "/" + value, file_list, target_ext);
    } else if (target_ext == "*" || target_ext == file_ext) {
      file_list.push_back(path + "/" + value);
    }
  }

  closedir(dir);
}

int zp3_parse_metadata(const std::string &song_path,
                       song_t &song) {
  // Open file
  mpg123_handle *m = mpg123_new(NULL, NULL);
  if (mpg123_open(m, song_path.c_str()) != MPG123_OK) {
    LOG_ERROR("Failed to open file: [%s]!", song_path.c_str());
    return -1;
  }
  mpg123_scan(m);

  // Check metadata
  int meta = mpg123_meta_check(m);
  if ((meta & MPG123_ID3) == false) {
    LOG_ERROR("Metadata doesn't seem to be ID3!");
    return -1;
  }

  // Get metadata
  mpg123_id3v1 *v1 = NULL;
  mpg123_id3v2 *v2 = NULL;
  if (mpg123_id3(m, &v1, &v2) != MPG123_OK) {
    LOG_ERROR("Failed to parse ID3 metadata!");
    return -1;
  }

  // Parse metadata
  if (v1 != NULL)  {
    song.file_path = song_path;
    song.title = std::string(v1->title);
    song.artist = std::string(v1->artist);
    song.album = std::string(v1->album);
    song.year = std::string(v1->year);
  } else if (v2 != NULL)  {
    song.file_path = song_path;
    song.title = (v2->title) ? std::string(v2->title->p) : "";
    song.artist = (v2->artist) ? std::string(v2->artist->p) : "";
    song.album = (v2->album) ? std::string(v2->album->p) : "";
    song.year = (v2->year) ? std::string(v2->year->p) : "";
  }

  return 0;
}

int zp3_load_library(const std::string &path) {
  std::vector<std::string> file_list;
  walkdir(path, file_list, "mp3");
  for (const auto &file : file_list) {
    song_t song;
    zp3_parse_metadata(file, song);
    printf("title: %s\n", song.title.c_str());
    printf("artist: %s\n", song.artist.c_str());
    printf("album: %s\n", song.album.c_str());
    printf("\n");
  }

  return 0;
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

int main(int argc, char **argv) {
  // Setup
  mpg123_init();  // Do this only once!
  zp3_t zp3;

  // zp3_load_library("/data/music");

  song_t song;
  zp3.song_path = "/data/music/Gorillaz_-_Demon_Days_(Remaster)_(2005)_FLAC/01 - Intro.mp3";
  zp3_parse_metadata(zp3.song_path, song);

  // zp3.song_path = "/data/music/great_success.mp3";
  // pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);
  // pthread_join(zp3.player_thread_id, NULL);

  return 0;
}
