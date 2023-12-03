#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "data.hpp"
#include "data_buffer.hpp"
#include "queue.hpp"
#include "utils.hpp"

#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>

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
        _last_request_time(std::chrono::system_clock::now()), _in_queue(in_queue) {}

  ~Connection() {
    auto ec = finish();
    if (ec) {
      std::cerr << "Finish Error: " << ec.message() << std::endl;
      std::exit(1);
    }
  }

  std::shared_ptr<Connection> get_shared_ptr() { return this->shared_from_this(); }

  const T& socket() const { return _socket; }
  bool is_connected() const { return _socket.is_open(); }
  bool is_timed_out() const {
    return std::chrono::system_clock::now() - _last_request_time > std::chrono::minutes(5);
  }

  bool closable() const { return is_timed_out() || !is_connected(); }

  void send(const message::Data& data);
  void receive(std::uint32_t connection_id);
  boost::system::error_code finish();

private:
  void commit(const message::Data& data);

  boost::system::error_code handle_message(std::uint32_t connection_id,
                                           boost::system::error_code ec,
                                           std::size_t bytes_transferred);

  T _socket;
  boost::asio::io_context& _io_context;
  boost::asio::streambuf _buffer;

  std::chrono::time_point<std::chrono::system_clock> _last_request_time;

  utils::Queue<message::Data>& _in_queue;
};

} // namespace connection
} // namespace web_server

// Implementation
namespace web_server {
namespace connection {

template <Socket T>
void Connection<T>::send(const message::Data& data) {
  boost::asio::async_write(
      _socket, boost::asio::buffer(reinterpret_cast<const void*>(data.data()), data.size()),
      [](boost::system::error_code ec, std::size_t bytes_transfered) {
        if (!ec) {
          std::cout << "Write " << bytes_transfered << " bytes!" << std::endl;
        } else {
          std::cerr << "Write Error: " << ec.message() << std::endl;
        }
      });
}

template <Socket T>
void Connection<T>::receive(std::uint32_t connection_id) {
  try {
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
                                  std::bind(&Connection::handle_message, get_shared_ptr(),
                                            connection_id, std::placeholders::_1,
                                            std::placeholders::_2));
  } catch (const std::bad_weak_ptr& e) {
    boost::asio::async_read_until(_socket, _buffer, "\r\n\r\n",
                                  std::bind(&Connection::handle_message, this, connection_id,
                                            std::placeholders::_1, std::placeholders::_2));
  }
}

template <Socket T>
boost::system::error_code Connection<T>::finish() {
  boost::system::error_code ec;
  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  _socket.close(ec);
  _buffer.consume(_buffer.size());
  return ec;
}

template <Socket T>
void Connection<T>::commit(const message::Data& data) {
  _in_queue.push(std::move(data));
}

template <Socket T>
boost::system::error_code Connection<T>::handle_message(std::uint32_t connection_id,
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
      if (line.find("Content-Length") != std::string::npos) {
        std::string_view key, value;
        utils::split_head(line, key, value);
        length += std::stoi(std::string(value));
        break;
      }
    }

    data.append((std::uint8_t*)_buffer.data().data(), length);
    commit(data);
    _buffer.consume(length);
    _last_request_time = std::chrono::system_clock::now();

    receive(connection_id);
  } else {
    if (ec == boost::asio::error::eof) {
      auto finish_ec = finish();
      if (finish_ec) {
        std::cerr << "Finish Error: " << finish_ec.message() << std::endl;
        std::exit(1);
      }
    } else
      std::cerr << "Read Error: " << ec.message() << std::endl;
  }

  return ec;
}

} // namespace connection
} // namespace web_server

#endif // CONNECTION_H_
