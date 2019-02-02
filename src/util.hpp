#ifndef ZP3_UTIL_HPP
#define ZP3_UTIL_HPP

#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>

#include <map>
#include <vector>

template <typename K, typename V>
std::vector<K> extract_keys(std::map<K, V> const& input_map) {
  std::vector<K> retval;
  for (auto const& element : input_map) {
    retval.push_back(element.first);
  }
  return retval;
}

template <typename T>
void pop_front(std::vector<T> &vec) {
  assert(!vec.empty());
  vec.front() = std::move(vec.back());
  vec.pop_back();
}

void walkdir(const std::string &path,
             std::vector<std::string> &file_list,
             const std::string &target_ext="*");

char getch();

#endif // ZP3_UTIL_HPP
