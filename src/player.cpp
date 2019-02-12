#include "player.hpp"

void player_init() {
  mpg123_init();  // Do this only once!
}

void *player_thread(void *arg) {
play_song:
  player_t *player = (player_t *) arg;
  player->player_state = PLAYER_PLAY;

  // Check song queue
  if (player->song_index >= player->song_queue.size()) {
    LOG_ERROR("No songs in play queue!");
    return nullptr;
  }

  // Initialize AO
  ao_initialize();
  int driver = ao_default_driver_id();

  // Initialize MPG123
  int err = 0;
  mpg123_handle *mh = mpg123_new(NULL, &err);
  size_t buffer_size = mpg123_outblock(mh);
  auto buffer = (unsigned char *) malloc(buffer_size * sizeof(unsigned char));

  // Open the file and get the decoding format
  const auto song = player->song_queue.at(player->song_index);
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
  mpg123_volume(mh, player->volume);
  player->song_length = mpg123_framelength(mh) * mpg123_tpf(mh);

  if (player->display != nullptr) {
    display_song(*player->display,
                 PLAYER_PLAY,
                 song,
                 0.0,
                 player->song_length);
  }

  // const size_t frame_length = mpg123_framelength(mh);
  size_t done;
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
    // Update song time
    player->song_time = (mpg123_tell(mh) / mpg123_spf(mh)) * mpg123_tpf(mh);
    if (player->display != nullptr) {
      display_song(*player->display,
                   player->player_state,
                   song,
                   player->song_time,
                   player->song_length);
    }

    // Pause?
    while (player->player_state == PLAYER_PAUSE);

    // Stop?
    if (player->player_state == PLAYER_STOP) {
      break;
    }

    // Keep playing
    ao_play(dev, (char *) buffer, done);

    // Set volume
    mpg123_volume(mh, player->volume);
  }

  // Print 100%
  if (player->display != nullptr) {
    display_song(*player->display,
                  player->player_state,
                  song,
                  player->song_length,
                  player->song_length);
  }

  // Clean up
  free(buffer);
  ao_close(dev);
  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();
  ao_shutdown();

  // Reset player
  // player->song_path = "";
  // player->player_state = PLAYER_STOP;
  player->player_is_dead = true;
  player->song_length = 0.0f;
  player->song_time = 0.0f;

  // Play next song if song queue is not finished
  if (player->player_state != PLAYER_STOP) {
    if ((player->song_index + 1) != player->song_queue.size()) {
      player->song_index++;
      goto play_song;
    }
  }

  return nullptr;
}

int player_play(player_t &player) {
  if (player.player_state != PLAYER_STOP) {
    player.player_state = PLAYER_STOP;
    player.thread.join();
  }
  std::thread t(player_thread, &player);
  player.thread = std::move(t);

  return 0;
}

void player_stop(player_t &player) {
  player.player_state = PLAYER_STOP;
  sleep(1);
}

void player_toggle_pause_play(player_t &player) {
  if (player.player_state == PLAYER_PLAY) {
    player.player_state = PLAYER_PAUSE;
  } else if (player.player_state == PLAYER_PAUSE) {
    player.player_state = PLAYER_PLAY;
  }
}

void player_volume_up(player_t &player) {
  player.volume += player.volume_delta;
  player.volume = (player.volume > 1.0) ? 1.0 : player.volume;
}

void player_volume_down(player_t &player) {
  player.volume -= player.volume_delta;
  player.volume = (player.volume < 0.0) ? 0.0 : player.volume;
}
