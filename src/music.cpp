#include "music.hpp"

bool song_comparator(const song_t &s1, const song_t &s2) {
  if (s1.artist != s2.artist) {
    return (s1.artist < s2.artist);
  }

  if (s1.album != s2.album) {
    return (s1.album < s2.album);
  }

  return (s1.track_number < s2.track_number);
}

void print_song(const song_t &song) {
  printf("title: %s\n", song.title.c_str());
  printf("artist: %s\n", song.artist.c_str());
  printf("album: %s\n", song.album.c_str());
  printf("year: %d\n", song.year);
  printf("track number: %d\n", song.track_number);
}
