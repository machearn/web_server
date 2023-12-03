#include "include/data.hpp"

namespace web_server {
namespace message {

Data::Data(std::size_t capacity)
    : _allocator(), _data(_allocator.allocate(capacity)), _size(0), _capacity(capacity),
      _connection_id(0) {}

Data::Data(std::size_t capacity, std::uint32_t connection_id)
    : _allocator(), _data(_allocator.allocate(capacity)), _size(0), _capacity(capacity),
      _connection_id(connection_id) {}

Data::Data(const std::uint8_t* data, std::size_t size, std::uint32_t connection_id)
    : _allocator(), _data(_allocator.allocate(size)), _size(size), _capacity(size),
      _connection_id(connection_id) {
  std::memcpy(_data, data, _size);
}

Data::Data(const Data& data)
    : _allocator(), _data(_allocator.allocate(data._size)), _size(data._size),
      _connection_id(data._connection_id) {
  std::memcpy(_data, data._data, _size);
}

Data& Data::operator=(const Data& data) {
  if (this != &data) {
    _allocator.deallocate(_data, _size);
    _data = _allocator.allocate(data._size);
    _size = data._size;
    _connection_id = data._connection_id;
    std::memcpy(_data, data._data, _size);
  }
  return *this;
}

Data::Data(Data&& data) {
  if (this == &data) {
    return;
  }

  _data = data._data;
  _size = data._size;
  _capacity = data._capacity;
  _connection_id = data._connection_id;

  data._data = nullptr;
  data._size = 0;
  data._capacity = 0;
  data._connection_id = 0;
}

Data& Data::operator=(Data&& data) {
  if (this == &data) {
    return *this;
  }

  if (_data != nullptr) {
    _allocator.deallocate(_data, _size);
    _data = nullptr;
  }

  _data = data._data;
  _size = data._size;
  _capacity = data._capacity;
  _connection_id = data._connection_id;

  data._data = nullptr;
  data._size = 0;
  data._capacity = 0;
  data._connection_id = 0;
  return *this;
}

Data::~Data() { clear(); }

void Data::reserve(std::size_t capacity) {
  if (_data == nullptr) {
    _data = _allocator.allocate(capacity);
    _capacity = capacity;
    return;
  }
  if (capacity <= _capacity) {
    return;
  }
  std::uint8_t* new_data = _allocator.allocate(capacity);
  std::memcpy(new_data, _data, _size);
  _allocator.deallocate(_data, _size);
  _data = new_data;
  _capacity = capacity;
}

void Data::clear() {
  if (_data != nullptr) {
    _allocator.deallocate(_data, _size);
  }
  _data = nullptr;
  _size = 0;
  _connection_id = 0;
  _capacity = 0;
}

void Data::append(const std::uint8_t* data, std::size_t size) {
  if (_data == nullptr) {
    reserve(size);
  }
  if (_size + size <= _capacity) {
    std::memcpy(_data + _size, data, size);
    _size += size;
  } else {
    reserve(_size + size);
    std::memcpy(_data + _size, data, size);
    _size += size;
  }
}

void Data::append(std::iostream& stream, std::size_t size) {
  if (_data == nullptr) {
    reserve(size);
  }
  if (_size + size <= _capacity) {
    stream.read(reinterpret_cast<char*>(_data + _size), size);
    _size += size;
  } else {
    reserve(_size + size);
    stream.read(reinterpret_cast<char*>(_data + _size), size);
    _size += size;
  }
}

} // namespace message
} // namespace web_server
