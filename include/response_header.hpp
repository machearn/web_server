#ifndef RESPONSE_HEADER_H_
#define RESPONSE_HEADER_H_

#include "data_view.hpp"
#include "utils.hpp"

#include <memory>
#include <string>

namespace web_server {
namespace message {
class ResponseHeader: public std::enable_shared_from_this<ResponseHeader> {
public:
  ResponseHeader() = default;
  ResponseHeader(std::uint32_t status_code, const std::string& status_message)
    : _status_code(status_code), _status_message(std::move(status_message)) {}

  ~ResponseHeader() = default;

  int parse(const DataView& data);
  std::size_t to_bytes(std::string& bytes) const;

  const std::string& version() const { return _version; }
  std::uint32_t status_code() const { return _status_code; }
  const std::string& status_message() const { return _status_message; }

  void set_version(const std::string& version) { _version = version; }
  void set_status_code(std::uint32_t status_code) { _status_code = status_code; }
  void set_status_message(const std::string& status_message) { _status_message = status_message; }

  bool contain(const std::string& key) const { return _headers.contains(key); }
  const std::string& get(const std::string& key) const { return _headers.at(key); }
  void set(const std::string& key, const std::string& value) { _headers[key] = value; }

private:
  std::string _version;
  std::uint32_t _status_code;
  std::string _status_message;

  std::unordered_map<std::string, std::string> _headers;
};

} // namespace message
} // namespace web_server

#endif // RESPONSE_HEADER_H_
