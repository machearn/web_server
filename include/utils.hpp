#ifndef UTILS_H_
#define UTILS_H_

#include <string>

namespace web_server {
namespace utils {


[[nodiscard]] int get_line(std::string_view data, std::string_view& line, int* start);

void split_line(std::string_view line, std::vector<std::string_view>& words);

void split_head(std::string_view line, std::string_view& key, std::string_view& value);


} // namespace utils
} // namespace web_server


#endif // UTILS_H_
