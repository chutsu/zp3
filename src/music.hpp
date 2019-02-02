#ifndef ZP3_MUSIC_HPP
#define ZP3_MUSIC_HPP

#include <map>
#include <set>
#include <vector>

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

bool song_comparator(const song_t &s1, const song_t &s2);
void print_song(const song_t &song);

#endif // ZP3_MUSIC_HPP
