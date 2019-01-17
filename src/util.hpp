#ifndef ZP3_UTIL_HPP
#define ZP3_UTIL_HPP

#include <assert.h>
#include <unistd.h>
#include <termios.h>

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
             const std::string &target_ext="*") {
  DIR *dir = opendir(path.c_str());
  if (dir == NULL) {
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string value(entry->d_name);
    if (value == "." || value == "..") {
      continue;
    }

    const auto ext_len = target_ext.length();
    const auto file_ext = value.substr(value.length() - ext_len, ext_len);
    if (entry->d_type == DT_DIR) {
      walkdir(path + "/" + value, file_list, target_ext);
    } else if (target_ext == "*" || target_ext == file_ext) {
      file_list.push_back(path + "/" + value);
    }
  }

  closedir(dir);
}

char getch() {
  char buf = 0;
  struct termios old = {0};

  if (tcgetattr(0, &old) < 0) {
    perror("tcsetattr()");
  }

  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0) {
    perror("tcsetattr ICANON");
  }
  if (read(0, &buf, 1) < 0) {
    perror ("read()");
  }
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0) {
    perror ("tcsetattr ~ICANON");
  }

  return (buf);
}

#endif // ZP3_UTIL_HPP
