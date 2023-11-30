#ifndef REQUEST_HEADER_H_
#define REQUEST_HEADER_H_

#include "data_view.hpp"
#include "utils.hpp"
#include "data_buffer.hpp"

#include <memory>
#include <string>

namespace web_server {
namespace message{

enum class Method {
  GET,
  POST,
  PUT,
  DELETE,
  HEAD,
  OPTIONS,
  TRACE,
  CONNECT,
  PATCH
};

class RequestHeader: public std::enable_shared_from_this<RequestHeader> {
public:
  RequestHeader() = default;
  ~RequestHeader() = default;

  int parse(const DataView& data);
  std::size_t to_bytes(std::string& bytes) const;

  const std::string& path() const {
    return _path;
  }

  void set_path(const std::string& path) {
    _path = path;
  }

  const std::string& version() const {
    return _version;
  }

  void set_version(const std::string& version) {
    _version = version;
  }

  const Method& method() const {
    return _method;
  }

  void set_method(const Method& method) {
    _method = method;
  }


  bool contain(const std::string& key) const {
    return _headers.find(key) != _headers.end();
  }

  const std::string& get(const std::string& key) const {
    return _headers.at(key);
  }

  void set(const std::string& key, const std::string& value) {
    _headers[key] = value;
  }

private:
  void parse_method(std::string_view method);

  [[nodiscard]] std::string method_to_string() const;

  std::string _path;
  std::string _version;
  Method _method;

  std::unordered_map<std::string, std::string> _headers;
};

} // namespace message
} // namespace web_server

#endif // REQUEST_HEADER_H_
