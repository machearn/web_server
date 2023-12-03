#include "include/utils.hpp"

namespace web_server {
namespace utils {

[[nodiscard]] int get_line(std::string_view data, std::string_view &line,
                           int *start) {
  int i = *start;
  while (data[i] != '\r' && data[i + 1] != '\n') {
    ++i;
  }

  if (i == *start) {
    *start = i + 2;
    return -1;
  }

  line = std::string_view(data.data() + *start, i - *start);
  *start = i + 2;
  return 0;
}

void split_line(std::string_view line, std::vector<std::string_view> &words) {
  int i = 0;
  while (line[i] != ' ') {
    ++i;
  }

  words.emplace_back(line.data(), i);
  int j = i + 1;
  while (line[j] != ' ') {
    ++j;
  }

  words.emplace_back(line.data() + i + 1, j - i - 1);
  words.emplace_back(line.data() + j + 1, line.size() - j - 1);
}

void split_head(std::string_view line, std::string_view &key,
                std::string_view &value) {
  int i = 0;
  while (line[i] != ':') {
    ++i;
  }

  key = std::string_view(line.data(), i);
  i++;
  while (line[i] == ' ') {
    ++i;
  }
  value = std::string_view(line.data() + i, line.size() - i);
}

} // namespace utils
} // namespace web_server
