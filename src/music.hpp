#ifndef ZP3_MUSIC_HPP
#define ZP3_MUSIC_HPP

struct song_t {
  std::string file_path;
  std::string title;
  std::string artist;
  std::string album;
  int year = -1;
  int track_number = -1;
};

typedef std::vector<song_t> songs_t;
typedef std::map<std::string, std::set<std::string>> artists_t;
typedef std::map<std::string, std::vector<song_t>> albums_t;

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

#endif // ZP3_MUSIC_HPP