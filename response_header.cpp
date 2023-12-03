#include "include/response_header.hpp"

namespace web_server {
namespace message {

int ResponseHeader::parse(const DataView& data) {
  int start = 0;
  int ret = 0;
  std::string_view line;

  std::string_view string_data(reinterpret_cast<const char*>(data.data()), data.size());

  ret = web_server::utils::get_line(string_data, line, &start);
  std::vector<std::string_view> words;
  web_server::utils::split_line(line, words);
  _version = std::string(words[0]);
  _status_code = std::stoi(std::string(words[1]));
  _status_message = std::string(words[2]);

  while ((ret = web_server::utils::get_line(string_data, line, &start)) != -1) {
    std::string_view key, value;
    web_server::utils::split_head(line, key, value);
    _headers.emplace(std::string(key), std::string(value));
  }

  return start;
}

std::size_t ResponseHeader::to_bytes(std::string& bytes) const {
  bytes += _version;
  bytes += " ";
  bytes += std::to_string(_status_code);
  bytes += " ";
  bytes += _status_message;
  bytes += "\r\n";

  for (const auto& [key, value] : _headers) {
    bytes += key;
    bytes += ": ";
    bytes += value;
    bytes += "\r\n";
  }

  bytes += "\r\n";

  return bytes.size();
}

} // namespace message
} // namespace web_server
