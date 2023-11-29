#ifndef DATA_H_
#define DATA_H_

#include <memory>
#include <cstring>
#include <iostream>

namespace web_server {
namespace message {

class Data {
public:
  Data() = default;
  Data(std::size_t capacity);
  Data(std::size_t capacity, std::uint32_t connection_id);
  Data(const std::uint8_t* data, std::size_t size, std::uint32_t connection_id);

  Data(const Data&);
  Data& operator=(const Data&);
  Data(Data&&);
  Data& operator=(Data&&);

  ~Data();

  const std::uint8_t* data() const { return _data; }
  const std::size_t& size() const { return _size; }
  const std::uint32_t& connection_id() const { return _connection_id; }
  const std::size_t& capacity() const { return _capacity; }

  void set_connection_id(std::uint32_t connection_id) { _connection_id = connection_id; }

  void reserve(std::size_t capacity);
  void append(const std::uint8_t *data, std::size_t size);
  void append(std::iostream& stream, std::size_t size);
  void clear();

private:
  std::allocator<std::uint8_t> _allocator;

  std::uint8_t* _data;
  std::size_t _size;
  std::size_t _capacity;
  std::uint32_t _connection_id;
};

} // namespace message
} // namespace web_server

#endif // DATA_H_
