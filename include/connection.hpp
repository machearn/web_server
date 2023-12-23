#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "data.hpp"
#include "data_buffer.hpp"
#include "logger.hpp"
#include "queue.hpp"
#include "utils.hpp"

#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

namespace web_server {
namespace connection {

template <typename T>
concept Socket = std::is_base_of<boost::asio::ip::tcp::socket, T>::value;

template <Socket T>
class Connection: public std::enable_shared_from_this<Connection<T>> {
public:
  Connection() = delete;
  Connection(boost::asio::io_context& io_context, T socket, utils::Queue<message::Data>& in_queue)
      : _socket(std::move(socket)), _io_context(io_context), _buffer(),
        _last_active_time(std::chrono::system_clock::now()), _in_queue(in_queue) {}

  ~Connection() {
    auto ec = finish();
    if (ec) {
      utils::Logger::logger().error("Connection Finish Error: " + ec.message());
      std::exit(1);
    }
  }

  std::shared_ptr<Connection> get_shared_ptr() { return this->shared_from_this(); }

  const T& socket() const { return _socket; }
  bool is_connected() const { return _socket.is_open(); }
  bool is_timed_out() const {
    return std::chrono::system_clock::now() - _last_active_time > std::chrono::minutes(5);
  }

  bool closable() const { return is_timed_out() || !is_connected(); }

  void send(const message::Data& data);
  void receive(std::uint32_t connection_id);
  boost::system::error_code finish();

private:
  void commit(const message::Data& data);

  boost::system::error_code handle_write(boost::system::error_code ec,
                                         std::size_t bytes_transfered);
  boost::system::error_code handle_read(std::uint32_t connection_id, boost::system::error_code ec,
                                        std::size_t bytes_transferred);

  T _socket;
  boost::asio::io_context& _io_context;
  boost::asio::streambuf _buffer;

  std::chrono::time_point<std::chrono::system_clock> _last_active_time;

  utils::Queue<message::Data>& _in_queue;
};

} // namespace connection
} // namespace web_server

// Implementation
namespace web_server {
namespace connection {

template <Socket T>
void Connection<T>::send(const message::Data& data) {
  utils::Logger::logger().info("Connection send data");
#ifdef DEBUG
  utils::Logger::logger().debug("Connection send data: " + data.to_string());
  utils::Logger::logger().debug("Connection send data size: " + std::to_string(data.size()));
#endif
  try {
    boost::asio::async_write(
        _socket, boost::asio::buffer(reinterpret_cast<const void*>(data.data()), data.size()),
        std::bind(&Connection::handle_write, get_shared_ptr(), std::placeholders::_1,
                  std::placeholders::_2));
  } catch (const std::bad_weak_ptr& e) {
    boost::asio::async_write(
        _socket, boost::asio::buffer(reinterpret_cast<const void*>(data.data()), data.size()),
        std::bind(&Connection::handle_write, this, std::placeholders::_1, std::placeholders::_2));
  }
}

template <Socket T>
void Connection<T>::receive(std::uint32_t connection_id) {
  utils::Logger::logger().info("Connection " + std::to_string(connection_id) + " receive data ");
  try {
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
                                  std::bind(&Connection::handle_read, get_shared_ptr(),
                                            connection_id, std::placeholders::_1,
                                            std::placeholders::_2));
  } catch (const std::bad_weak_ptr& e) {
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
                                  std::bind(&Connection::handle_read, this, connection_id,
                                            std::placeholders::_1, std::placeholders::_2));
  }
}

template <Socket T>
boost::system::error_code Connection<T>::finish() {
  boost::system::error_code ec;
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  _socket.close(ec);
  _buffer.consume(_buffer.size());
  utils::Logger::logger().info("Connection Closed");
  return ec;
}

template <Socket T>
void Connection<T>::commit(const message::Data& data) {
  _in_queue.push(std::move(data));
}

template <Socket T>
boost::system::error_code Connection<T>::handle_read(std::uint32_t connection_id,
                                                     boost::system::error_code ec,
                                                     std::size_t bytes_transferred) {
  if (!ec) {
    message::Data data{};
    data.set_connection_id(connection_id);

    message::DataBuffer data_buffer{(char*)_buffer.data().data(), bytes_transferred};

    std::istream is(dynamic_cast<std::streambuf*>(&data_buffer));
    std::string line{};
    std::uint32_t length = bytes_transferred;
    while (std::getline(is, line) && line != "\r") {
#ifdef DEBUG
      utils::Logger::logger().debug("Connection read line: " + line);
#endif
      if (line.find("Content-Length") != std::string::npos) {
        std::string_view key, value;
        utils::split_head(line, key, value);
        length += std::stoi(std::string(value));
#ifdef DEBUG
        utils::Logger::logger().debug("Connection Read Content-Length: " + std::string(value));
        utils::Logger::logger().debug("Connection Total Length: " + std::to_string(length));
#endif
        break;
      }
    }

    data.append((std::uint8_t*)_buffer.data().data(), length);
#ifdef DEBUG
    utils::Logger::logger().debug(
        "Connection Read: " + std::string{reinterpret_cast<const char*>(data.data()), data.size()});
#endif
    commit(data);
    _buffer.consume(length);
    _last_active_time = std::chrono::system_clock::now();
    utils::Logger::logger().info("Connection Read " + std::to_string(length) + " bytes");

    receive(connection_id);
  } else {
    if (ec == boost::asio::error::eof) {
      utils::Logger::logger().warning("Connection End of File, trying to close");
      auto finish_ec = finish();
      if (finish_ec) {
        utils::Logger::logger().error("Connection Finish Error: " + finish_ec.message());
        std::exit(1);
      }
    } else {
      utils::Logger::logger().error("Connection Read Error: " + ec.message());
    }
  }

  return ec;
}

template <Socket T>
boost::system::error_code Connection<T>::handle_write(boost::system::error_code ec,
                                                      std::size_t bytes_transfered) {
  _last_active_time = std::chrono::system_clock::now();
  if (!ec) {
    utils::Logger::logger().info("Connection Write " + std::to_string(bytes_transfered) + " bytes");
  } else {
    utils::Logger::logger().error("Connection Write Error: " + ec.message());
  }
  return ec;
}

} // namespace connection
} // namespace web_server

#endif // CONNECTION_H_
