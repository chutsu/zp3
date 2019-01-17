#ifndef ZP3_UTIL_HPP
#define ZP3_UTIL_HPP

template <typename K, typename V>
std::vector<K> extract_keys(std::map<K, V> const& input_map) {
  std::vector<K> retval;
  for (auto const& element : input_map) {
    retval.push_back(element.first);
  }
  return retval;
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

#endif // ZP3_UTIL_HPP
