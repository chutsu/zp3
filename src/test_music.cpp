#include "test.hpp"
#include "music.hpp"

int test_song_parse_metadata() {
  const auto song_path = TEST_MUSIC_LIBRARY "/album1/1-apple.mp3";
  song_t song;
  song_parse_metadata(song, song_path);
  // song_print(song);

  CHECK(song.title == "Apple");
  CHECK(song.artist == "Bob Dylan");
  CHECK(song.album == "ALBUM1");
  CHECK(song.year == 2018);
  CHECK(song.track_number == 1);

  return 0;
}

int test_music_load_library() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  CHECK(music.artists.size() == 2);
  CHECK(music.albums.size() == 3);
  CHECK(music.songs.size() == 15);

  return 0;
}

int test_music_filter_songs() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  // Add artist filter
  {
    const auto target_artist = "Bob Dylan";
    const auto songs = music_filter_songs(music, target_artist);
    for (const auto &song : songs) {
      CHECK(song.artist == "Bob Dylan");
    }
  }

  // Add artist filter
  {
    const auto target_artist = "Michael Jackson";
    const auto target_album = "ALBUM3";
    const auto songs = music_filter_songs(music, target_artist, target_album);
    for (const auto &song : songs) {
      CHECK(song.artist == "Michael Jackson");
      CHECK(song.album == "ALBUM3");
    }
  }

  return 0;
}

int test_music_filter_albums() {
  music_t music;
  music_load_library(music, TEST_MUSIC_LIBRARY);

  // No filter
  auto albums = music_filter_albums(music);
  CHECK(albums.size() > 0);

  // Apply artist filter
  const auto target_artist = "Michael Jackson";
  albums = music_filter_albums(music, target_artist);
  CHECK(albums.size() == 2);
  CHECK(albums.at(0) == "ALBUM2");
  CHECK(albums.at(1) == "ALBUM3");

  return 0;
}

int main(int argc, char **argv) {
  RUN_TEST(test_song_parse_metadata);
  RUN_TEST(test_music_load_library);
  RUN_TEST(test_music_filter_songs);
  RUN_TEST(test_music_filter_albums);

  return 0;
}
