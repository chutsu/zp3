#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>

#include <ao/ao.h>
#include <mpg123.h>

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

void safe_print(char* name, char *data, size_t size) {
  // char safe[31];
  // if (size>30) return;
  //
  // memcpy(safe, data, size);
  // safe[size] = 0;
  // printf("%s: %s\n", name, safe);
}

void print_id3v1(mpg123_id3v1 *v1) {
  printf("\n==== ID3v1 ====\n");

  // safe_print("Title", v1->title, sizeof(v1->title));
  // safe_print("Artist", v1->artist, sizeof(v1->artist));
  // safe_print("Album", v1->album, sizeof(v1->album));
  // safe_print("Year", v1->year, sizeof(v1->year));
  // safe_print("Comment", v1->comment, sizeof(v1->comment));
  printf("Genre: %i", v1->genre);
}

void print_lines(const char* prefix, mpg123_string *inlines) {
  size_t i;
  int hadcr = 0, hadlf = 0;
  char *lines = NULL;
  char *line = NULL;
  size_t len = 0;

  if (inlines != NULL && inlines->fill) {
    lines = inlines->p;
    len = inlines->fill;
  }
  else return;

  line = lines;
  for (i=0; i<len; ++i) {
    if (lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0) {
      char save = lines[i];  /* saving, changing, restoring a byte in the data */
      if (save == '\n') ++hadlf;
      if (save == '\r') ++hadcr;
      if ((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) line = (char *) "";

      if (line) {
        lines[i] = 0;
        printf("%s%s\n", prefix, line);
        line = NULL;
        lines[i] = save;
      }
    } else {
      hadlf = hadcr = 0;
      if (line == NULL) line = lines+i;
    }
  }
}

void print_id3v2(mpg123_id3v2 *v2) {
  printf("\n==== ID3v2 ====\n");
  print_lines("Title: ", v2->title);
  print_lines("Artist: ", v2->artist);
  print_lines("Album: ", v2->album);
  print_lines("Year: ", v2->year);
  print_lines("Comment: ", v2->comment);
  print_lines("Genre: ", v2->genre);
}

void *player_thread(void *arg) {
  zp3_t *zp3 = (zp3_t *) arg;
  zp3->player_state = PLAY;

  // Initialize AO
  ao_initialize();
  int driver = ao_default_driver_id();

  // Initialize MPG123
  mpg123_init();
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
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
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
  zp3_t zp3;
  zp3.song_path = "/data/music/Gorillaz_-_Demon_Days_(Remaster)_(2005)_FLAC/01 - Intro.mp3";
  mpg123_init();

  std::cout << zp3.song_path << std::endl;
  mpg123_handle *m = mpg123_new(NULL, NULL);
  if (mpg123_open(m, zp3.song_path.c_str()) != MPG123_OK) {
    return -1;
  }
  mpg123_scan(m);
  int meta = mpg123_meta_check(m);

  mpg123_id3v1 *v1;
  mpg123_id3v2 *v2;
  if (meta & MPG123_ID3 && mpg123_id3(m, &v1, &v2) == MPG123_OK) {
    if (v1 != NULL) print_id3v1(v1);
    if (v2 != NULL) print_id3v2(v2);
  }

  // pthread_create(&zp3.player_thread_id, NULL, player_thread, &zp3);
  // pthread_join(zp3.player_thread_id, NULL);

  return 0;
}
