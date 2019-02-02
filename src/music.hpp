#ifndef ZP3_MUSIC_HPP
#define ZP3_MUSIC_HPP

#include <map>
#include <set>
#include <vector>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

#include "log.hpp"
#include "util.hpp"

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

struct music_t {
  songs_t songs;
  artists_t artists;
  albums_t albums;
};

bool song_comparator(const song_t &s1, const song_t &s2);
void song_print(const song_t &song);
int song_parse_metadata(song_t &song, const std::string &song_path);
int songs_parse_metadata(std::vector<song_t> &songs,
                         const std::vector<std::string> &song_paths);

int music_load_library(music_t &music, const std::string &path);
songs_t music_filter_songs(const music_t &zp3,
                           const std::string &target_artist = "",
                           const std::string &target_album = "");
std::vector<std::string> music_filter_albums(const music_t &music,
                                             const std::string &target_artist = "");

#endif // ZP3_MUSIC_HPP
