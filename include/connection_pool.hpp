#ifndef CONNECTION_POOL_H_
#define CONNECTION_POOL_H_

#include "connection.hpp"

#include <memory>
#include <set>
#include <vector>

namespace web_server {
namespace connection {

template <Socket T>
using ConstConnectionPtr = std::shared_ptr<const Connection<T>>;
template <Socket T>
using ConnectionPtr = std::shared_ptr<Connection<T>>;
using TcpConnection = Connection<boost::asio::ip::tcp::socket>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

template <Socket T>
class ConnectionPool {
public:
  ConnectionPool();
  ConnectionPool(std::uint32_t max_connections);

  ~ConnectionPool() = default;

  std::int32_t add(const ConnectionPtr<T> connection);
  std::int32_t emplace(boost::asio::io_context& io_context, T socket,
                       utils::Queue<message::Data>& in_queue);

  void erase(std::int32_t id) {
    _connections[id] = nullptr;
    _available_ids.insert(id);
  }

  ConstConnectionPtr<T> get_connection(std::int32_t id) const {
    if (id >= _max_connections) {
      return nullptr;
    }
    return _connections[id];
  }
  ConnectionPtr<T> get_connection(std::int32_t id) {
    if (id >= _max_connections) {
      return nullptr;
    }
    return _connections[id];
  }

  void erase_all();
  void clear();

  std::size_t erase_unavaliable();

  bool is_empty() const { return _available_ids.size() == _max_connections; }
  bool is_full() const { return _available_ids.empty(); }
  std::uint32_t size() const { return _max_connections - _available_ids.size(); }
  std::uint32_t max_size() const { return _max_connections; }

private:
  std::uint32_t _max_connections;
  std::vector<ConnectionPtr<T>> _connections;
  std::set<std::int32_t> _available_ids;
};

} // namespace connection
} // namespace web_server

// Implementation
namespace web_server {
namespace connection {

template <Socket T>
ConnectionPool<T>::ConnectionPool()
    : _max_connections(std::numeric_limits<std::uint16_t>::max()),
      _connections(_max_connections, nullptr) {
  for (std::uint32_t i = 0; i < _max_connections; ++i) {
    _available_ids.insert(i);
  }
}

template <Socket T>
ConnectionPool<T>::ConnectionPool(std::uint32_t max_connections)
    : _max_connections(max_connections), _connections(max_connections, nullptr) {
  for (std::uint32_t i = 0; i < _max_connections; ++i) {
    _available_ids.insert(i);
  }
}

template <Socket T>
std::int32_t ConnectionPool<T>::add(const ConnectionPtr<T> connection) {
  if (_available_ids.empty() && erase_unavaliable() == 0) {
    return -1;
  }

  auto id = *_available_ids.begin();
  _available_ids.erase(id);
  _connections[id] = connection;
  return id;
}

template <Socket T>
std::int32_t ConnectionPool<T>::emplace(boost::asio::io_context& io_context, T socket,
                                        utils::Queue<message::Data>& in_queue) {
  if (_available_ids.empty() && erase_unavaliable() == 0) {
    return -1;
  }

  auto id = *_available_ids.begin();
  _available_ids.erase(id);
  _connections[id] = std::make_shared<Connection<T>>(io_context, std::move(socket), in_queue);
  return id;
}

template <Socket T>
void ConnectionPool<T>::erase_all() {
  std::fill(_connections.begin(), _connections.end(), nullptr);

  for (std::uint32_t i = 0; i < _max_connections; ++i) {
    _available_ids.insert(i);
  }
}

template <Socket T>
void ConnectionPool<T>::clear() {
  _connections.clear();
  _available_ids.clear();
}

template <Socket T>
std::size_t ConnectionPool<T>::erase_unavaliable() {
  std::size_t count = 0;
  for (auto i = 0u; i < _max_connections; ++i) {
    if (_connections[i] != nullptr && _connections[i]->closable()) {
      _connections[i] = nullptr;
      _available_ids.insert(i);
      ++count;
    }
  }
  return count;
}

} // namespace connection
} // namespace web_server

#endif // CONNECTION_POOL_H_
