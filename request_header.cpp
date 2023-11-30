#include "include/request_header.hpp"

namespace web_server {
namespace message {

int RequestHeader::parse(const DataView& data) {
  int start = 0;
  int ret = 0;
  std::string_view line;

  std::string_view string_data(reinterpret_cast<const char*>(data.data()), data.size());

  ret = web_server::utils::get_line(string_data, line, &start);
  std::vector<std::string_view> words;
  web_server::utils::split_line(line, words);
  parse_method(words[0]);
  _path = std::string(words[1]);
  _version = std::string(words[2]);

  while ((ret = web_server::utils::get_line(string_data, line, &start)) != -1) {
    std::string_view key, value;
    web_server::utils::split_head(line, key, value);
    _headers.emplace(std::string(key), std::string(value));
  }

  return start;
}

std::size_t RequestHeader::to_bytes(std::string& bytes) const {
  bytes += method_to_string();
  bytes += " ";
  bytes += _path;
  bytes += " ";
  bytes += _version;
  bytes += "\r\n";

  for (const auto& [key, value]: _headers) {
    bytes += key;
    bytes += ": ";
    bytes += value;
    bytes += "\r\n";
  }

  bytes += "\r\n";

  return bytes.size();
}

void RequestHeader::parse_method(std::string_view method) {
  if (method == "GET") {
    _method = Method::GET;
  } else if (method == "POST") {
    _method = Method::POST;
  } else if (method == "PUT") {
    _method = Method::PUT;
  } else if (method == "DELETE") {
    _method = Method::DELETE;
  } else if (method == "HEAD") {
    _method = Method::HEAD;
  } else if (method == "OPTIONS") {
    _method = Method::OPTIONS;
  } else if (method == "TRACE") {
    _method = Method::TRACE;
  } else if (method == "CONNECT") {
    _method = Method::CONNECT;
  } else if (method == "PATCH") {
    _method = Method::PATCH;
  } else {
    throw std::runtime_error("Unknown method");
  }
}

[[nodiscard]]
std::string RequestHeader::method_to_string() const {
  std::string ret;
  switch (_method) {
    case Method::GET:
      ret = "GET";
      break;
    case Method::POST:
      ret = "POST";
      break;
    case Method::PUT:
      ret = "PUT";
      break;
    case Method::DELETE:
      ret = "DELETE";
      break;
    case Method::HEAD:
      ret = "HEAD";
      break;
    case Method::OPTIONS:
      ret = "OPTIONS";
      break;
    case Method::TRACE:
      ret = "TRACE";
      break;
    case Method::CONNECT:
      ret = "CONNECT";
      break;
    case Method::PATCH:
      ret = "PATCH";
      break;
    default:
      throw std::runtime_error("Unknown method");
  }

  return ret;
}

} // namespace message
} // namespace web_server
