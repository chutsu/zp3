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

void song_print(const song_t &song) {
  printf("title: %s\n", song.title.c_str());
  printf("artist: %s\n", song.artist.c_str());
  printf("album: %s\n", song.album.c_str());
  printf("year: %d\n", song.year);
  printf("track number: %d\n", song.track_number);
}

int song_parse_metadata(song_t &song, const std::string &song_path) {
  TagLib::FileRef meta(song_path.c_str());
  if (!meta.isNull() && meta.tag()) {
    TagLib::Tag *tag = meta.tag();
    song.file_path = song_path;
    song.title = tag->title().toCString();
    song.artist = tag->artist().toCString();
    song.album = tag->album().toCString();
    song.year = tag->year();
    song.track_number = tag->track();

    if (song.title == "" || song.artist == "") {
      return -1;
    }
  } else {
    return -1;
  }

  return 0;
}

int songs_parse_metadata(std::vector<song_t> &songs,
                         const std::vector<std::string> &song_paths) {
  for (const auto &song_path: song_paths) {
    song_t song;
    if (song_parse_metadata(song, song_path) == 0) {
      songs.push_back(song);
    }
  }

  return 0;
}

int music_load_library(music_t &music, const std::string &path) {
  // Parse all songs
  std::vector<std::string> file_list;
  walkdir(path, file_list, "mp3");
  music.songs.clear();
  songs_parse_metadata(music.songs, file_list);
  std::sort(music.songs.begin(), music.songs.end(), song_comparator);
  if (music.songs.size() == 0) {
    LOG_ERROR("No songs found at [%s]!", path.c_str());
    return -1;
  }

  // Get all artists
  music.artists.clear();
  for (const auto &song : music.songs) {
    music.artists[song.artist].insert(song.album);
  }

  // Get all albums
  music.albums.clear();
  for (const auto &song : music.songs) {
    music.albums[song.album].push_back(song);
  }

  return 0;
}

std::vector<song_t> music_filter_songs(const music_t &music,
                                       const std::string &target_artist,
                                       const std::string &target_album) {
  std::vector<song_t> songs;

  if (target_artist == "" && target_album == "") {
    songs = music.songs;

  } else {
    for (const auto &song : music.songs) {
      bool add = true;
      if (target_artist != "" && target_artist != song.artist) {
        add = false;
      }
      if (target_album != "" && target_album != song.album) {
        add = false;
      }
      if (add) {
        songs.push_back(song);
      }
    }
  }

  return songs;
}

std::vector<std::string> music_filter_albums(const music_t &music,
                                             const std::string &target_artist) {
  auto keys = extract_keys<std::string, std::vector<song_t>>(music.albums);
  std::vector<std::string> albums;

  if (target_artist == "") {
    albums = keys;

  } else {
    for (const auto &album: keys) {
      const auto song = music.albums.at(album).at(0);
      if (target_artist == song.artist) {
        albums.push_back(album);
      }
    }
  }

  return albums;
}
