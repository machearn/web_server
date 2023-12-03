#include "include/response.hpp"

namespace web_server {
namespace message {

Response::Response(const DataView& data) {
  int start = _header.parse(data);
  auto sub_data = data.subdata(start, std::string::npos);

  _body = std::string((char*)sub_data.data(), sub_data.size());
}

} // namespace message
} // namespace web_server
