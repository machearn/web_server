#include "include/data_view.hpp"

namespace web_server {
namespace message {

const DataView DataView::subdata(std::size_t start, std::size_t count) const {
  if (count == std::string::npos) {
    count = _size - start;
  }
  if (count > _size - start) {
    count = _size - start;
  }
  return DataView(_data + start, count, _connection_id);
}

} // namespace message
} // namespace web_server
