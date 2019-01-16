#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <ao/ao.h>
#include <mpg123.h>

// ZP3 STATES
#define MENU 0
#define PLAY 1
#define STOP 2
#define PAUSE 3

struct zp3_t {
  int state = MENU;
  double volume = 0.3;
  double min_volume = 0.0;
  double max_volume = 1.0;
  std::string song_path;

  pthread_t player_thread_id;
};

void *player_thread(void *arg) {
  zp3_t *zp3 = (zp3_t *) arg;
  zp3->state = PLAY;

  // Initialize AO
  ao_initialize();
  int driver = ao_default_driver_id();

  // Initialize MPG123
  mpg123_init();
  int err;
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
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
    // Pause?
    while (zp3->state == PAUSE);

    // Stop?
    if (zp3->state == STOP) {
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
}

int main(int argc, char **argv) {
  zp3_t zp3;
  zp3.song_path = "/data/music/great_success.mp3";

  pthread_create(&zp3.player_thread_id, NULL, player_thread, &zp3);
  pthread_join(zp3.player_thread_id, NULL);

  return 0;
}
