#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>

#include <ao/ao.h>
#include <mpg123.h>

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
  int state = MENU;
  double volume = 0.3;
  double min_volume = 0.0;
  double max_volume = 1.0;
  std::string song_path;

  pthread_t player_thread_id;
  int player_state = STOP;
  bool player_is_dead = false;
};

struct song_t {
  std::string file_path;
  std::string title;
  std::string artist;
  std::string album;
  std::string year;
};

int zp3_parse_metadata(const std::string &song_path) {
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
  song_t song;
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

  std::cout << song.title << std::endl;
  std::cout << song.artist << std::endl;
  std::cout << song.album << std::endl;
  std::cout << song.year << std::endl;

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
  size_t done;
  mpg123_volume(mh, zp3->volume);

  printf("nb samples: %d\n", mpg123_length(mh));
  printf("framelength: %d\n", mpg123_framelength(mh));
  printf("samples per frame: %f\n", mpg123_spf(mh));
  printf("seconds per frame: %f\n", mpg123_tpf(mh));
  printf("seconds: %f\n", mpg123_framelength(mh) * mpg123_tpf(mh));

  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
    printf("sample id: %d\n", mpg123_tell(mh));
    printf("frame id: %d\n", mpg123_tell(mh) / mpg123_spf(mh));

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

  zp3->player_is_dead = true;
}

int main(int argc, char **argv) {
  // Setup
  mpg123_init();  // Do this only once!
  zp3_t zp3;

  // zp3.song_path = "/data/music/Gorillaz_-_Demon_Days_(Remaster)_(2005)_FLAC/01 - Intro.mp3";
  // int retval = zp3_parse_metadata(zp3.song_path);

  zp3.song_path = "/data/music/great_success.mp3";
  pthread_create(&zp3.player_thread_id, NULL, zp3_player_thread, &zp3);
  pthread_join(zp3.player_thread_id, NULL);

  return 0;
}
