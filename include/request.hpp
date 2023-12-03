#ifndef REQUEST_H_
#define REQUEST_H_

#include "request_header.hpp"

#include <memory>
#include <string>

namespace web_server {
namespace message {

class Request: public std::enable_shared_from_this<Request> {
public:
  Request() = default;
  Request(const RequestHeader& header, const std::string& body)
      : _body(std::move(body)), _header(std::move(header)) {}
  Request(const DataView& data);

  ~Request() = default;

  const std::string& body() const { return _body; }
  const RequestHeader& header() const { return _header; }

  void set_body(const std::string& body) { _body = body; }
  void set_header(const RequestHeader& header) { _header = header; }

  void to_bytes(std::string& data) const {
    _header.to_bytes(data);
    data += _body;
  }

private:
  std::string _body;
  RequestHeader _header;
};

} // namespace message
} // namespace web_server

#endif // REQUEST_H_
