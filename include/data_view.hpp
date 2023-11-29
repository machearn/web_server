#ifndef DATA_VIEW_H_
#define DATA_VIEW_H_

#include "data.hpp"

#include <string>

namespace web_server {
namespace message {

class DataView {
public:
  DataView() = delete;
  DataView(const std::uint8_t* data, const std::size_t size, const std::uint32_t connection_id)
    : _data(data), _size(size), _connection_id(connection_id) {}
  DataView(const Data& data)
    : _data(const_cast<std::uint8_t*>(data.data())), _size(data.size()), _connection_id(data.connection_id()) {}
  DataView(const DataView& other) = default;
  DataView(DataView&& other) = default;
  ~DataView() = default;

  const std::uint8_t* data() const { return _data; }
  const std::size_t& size() const { return _size; }
  const std::uint32_t& connection_id() const { return _connection_id; }
  const DataView subdata(std::size_t start, std::size_t count=std::string::npos) const;

private:
  const std::uint8_t* _data;
  const std::size_t _size;
  const std::uint32_t _connection_id;
};

}
}

#endif // DATA_VIEW_H_
