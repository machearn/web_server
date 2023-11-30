#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "response_header.hpp"

#include <memory>
#include <string>

namespace web_server {
namespace message{

class Response: public std::enable_shared_from_this<Response> {
public:
  Response() = default;
  Response(const ResponseHeader& header, const std::string& body)
    : _body(std::move(body)), _header(std::move(header)) {}
  Response(const DataView& data);

  ~Response() = default;

  const std::string& body() const { return _body; }
  const ResponseHeader& header() const { return _header; }

  void set_body(const std::string& body) { _body = body; }
  void set_header(const ResponseHeader& header) { _header = header; }

  void to_bytes(std::string& data) const {
    _header.to_bytes(data);
    data += _body;
  }

private:
  std::string _body;
  ResponseHeader _header;
};

} // namespace message
} // namespace web_server

#endif // RESPONSE_H_
