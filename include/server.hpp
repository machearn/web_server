/*
 * Server class
 * This class is the main class of the web server.
 * It is responsible for accepting connections and
 * dispatching requests to the corresponding handlers.
 * It also manages the connection pool and thread pool.
 *
 * But we can only have one server running at a time,
 * since only one _acceptor can be created for each port.
 * Therefore, TODO: we need to support multiple servers
 */
#ifndef SERVER_H_
#define SERVER_H_

#include "connection_pool.hpp"
#include "data.hpp"
#include "logger.hpp"
#include "queue.hpp"
#include "thread_pool.hpp"

#include <boost/asio.hpp>
#include <functional>
#include <string>

namespace web_server {

using TcpSocket = boost::asio::ip::tcp::socket;
using TcpConnectionPool = connection::ConnectionPool<TcpSocket>;
using TcpConnectionPtr = connection::ConnectionPtr<TcpSocket>;

template <typename T>
class Server {
public:
  Server() = delete;
  Server(std::uint16_t port)
      : _io_context(), _port(port),
        _acceptor(_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        _in_queue(), _out_queue(), _receive_thread_pool(4), _send_thread_pool(4),
        _listen_thread_pool(4), _connection_pool(20) {}
  Server(const Server&) = delete;
  Server(Server&&) = delete;
  Server& operator=(const Server&) = delete;
  Server& operator=(Server&&) = delete;
  virtual ~Server() { stop(); }

  void start();
  void stop();

  TcpConnectionPool& get_connection_pool() { return _connection_pool; }

  void wait_for_connection();
  std::uint32_t fetch_data(std::uint32_t max_count = -1);
  std::uint32_t deliver_data(std::uint32_t max_count = -1);

private:
  void handle_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket socket);
  void handle_send(std::uint32_t connection_id, const message::Data& data);
  message::Data handle_request(std::uint32_t connection_id, const message::Data& data);

  std::uint16_t _port;

  boost::asio::io_context _io_context;
  std::thread _io_context_thread;

  boost::asio::ip::tcp::acceptor _acceptor;

  utils::Queue<message::Data> _in_queue;
  utils::Queue<message::Data> _out_queue;

  std::thread _fetch_thread;
  std::thread _deliver_thread;

  bool _fetch_thread_running{false};
  bool _deliver_thread_running{false};

  std::mutex _fetch_mutex;
  std::mutex _deliver_mutex;

  thread::ThreadPool _receive_thread_pool;
  thread::ThreadPool _send_thread_pool;

  thread::ThreadPool _listen_thread_pool;

  TcpConnectionPool _connection_pool;
};

} // namespace web_server

// implementation of Server class
namespace web_server {

template <typename T>
void Server<T>::start() {
  try {
    utils::Logger::logger().info("Server::Starting Server");
    wait_for_connection();

    utils::Logger::logger().info("Server::Create io_context thread.");
    _io_context_thread = std::thread([this]() { _io_context.run(); });

    utils::Logger::logger().info("Server::Create fetch thread.");
    _fetch_thread_running = true;
    _fetch_thread = std::thread([this]() {
      while (true) {
        fetch_data(5);
        std::scoped_lock<std::mutex> lock{_fetch_mutex};
        if (!_fetch_thread_running) {
          break;
        }
      }
    });

    utils::Logger::logger().info("Server::Create deliver thread.");
    _deliver_thread_running = true;
    _deliver_thread = std::thread([this]() {
      while (true) {
        deliver_data(5);
        std::scoped_lock<std::mutex> lock{_deliver_mutex};
        if (!_deliver_thread_running) {
          break;
        }
      }
    });

    utils::Logger::logger().info("Server::Server listening on " + std::to_string(_port) + ".");
  } catch (std::exception& e) {
    utils::Logger::logger().error(e.what());
    stop();
  }
}

template <typename T>
void Server<T>::stop() {
  utils::Logger::logger().info("Server::Stopping Server.");

  utils::Logger::logger().info("Server::Destroy thread pools.");
  _receive_thread_pool.destroy();
  _send_thread_pool.destroy();
  _listen_thread_pool.destroy();

  utils::Logger::logger().info("Server::Destroy fetch thread.");
  {
    std::scoped_lock<std::mutex> lock{_fetch_mutex};
    _fetch_thread_running = false;
  }
  if (_fetch_thread.joinable()) {
    _fetch_thread.join();
  }

  utils::Logger::logger().info("Server::Destroy deliver thread.");
  {
    std::scoped_lock<std::mutex> lock{_deliver_mutex};
    _deliver_thread_running = false;
  }
  if (_deliver_thread.joinable()) {
    _deliver_thread.join();
  }

  utils::Logger::logger().info("Server::Destroy io_context thread.");
  _io_context.stop();
  if (_io_context_thread.joinable()) {
    _io_context_thread.join();
  }

  utils::Logger::logger().info("Server::Close acceptor.");
  _acceptor.close();

  utils::Logger::logger().info("Server::Clear connection pool.");
  _connection_pool.erase_all();
}

template <typename T>
void Server<T>::wait_for_connection() {
  utils::Logger::logger().info("Server::Waiting for connection...");
  _acceptor.async_accept(
      std::bind(&Server::handle_accept, this, std::placeholders::_1, std::placeholders::_2));
}

template <typename T>
std::uint32_t Server<T>::fetch_data(std::uint32_t max_count) {
  std::uint32_t count = 0;
  while (count < max_count && !_in_queue.empty()) {
    message::Data data = _in_queue.pop();
    auto ret = _receive_thread_pool.submit([this, data]() {
#ifdef DEBUG
      utils::Logger::logger().debug("Server::Fetched data: " + data.to_string());
      utils::Logger::logger().debug("Server::InQueue remain: " + std::to_string(_in_queue.size()));
#endif
      auto res = handle_request(data.connection_id(), data);
      _out_queue.push(res);
    });
    ++count;
  }
  if (count > 0) {
    utils::Logger::logger().info("Server::Fetched " + std::to_string(count) + " data.");
  }
  return count;
}

template <typename T>
std::uint32_t Server<T>::deliver_data(std::uint32_t max_count) {
  std::uint32_t count = 0;
  while (count < max_count && !_out_queue.empty()) {
    message::Data data = _out_queue.pop();
    auto ret = _send_thread_pool.submit([this, data]() {
#ifdef DEBUG
      utils::Logger::logger().debug("Server::Delivering data: " + data.to_string());
      utils::Logger::logger().debug("Server::OutQueue remain: " +
                                    std::to_string(_out_queue.size()));
#endif
      handle_send(data.connection_id(), data);
    });
    ++count;
  }
  if (count > 0) {
    utils::Logger::logger().info("Server::Delivered " + std::to_string(count) + " data.");
  }
  return count;
}

template <typename T>
void Server<T>::handle_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
  if (!ec) {
    utils::Logger::logger().info("Server::New connection accepted.");
    auto id = _connection_pool.emplace(_io_context, std::move(socket), _in_queue);
    if (id < 0) {
      utils::Logger::logger().error("Server::Connection pool is full.");
    } else {
      utils::Logger::logger().info("Server::Created Connection id: " + std::to_string(id));
      auto ret = _listen_thread_pool.submit(
          [this, id]() { _connection_pool.get_connection(id)->receive(id); });
    }
  } else {
    utils::Logger::logger().error(ec.message());
  }
  wait_for_connection();
}

template <typename T>
void Server<T>::handle_send(std::uint32_t connection_id, const message::Data& data) {
  utils::Logger::logger().info("Server::Handling response.");
  utils::Logger::logger().info("Server::Reveal connection id: " + std::to_string(connection_id) +
                               ".");
  auto connection = _connection_pool.get_connection(connection_id);
  if (connection) {
    connection->send(data);
  } else {
    utils::Logger::logger().error("Server::Connection id: " + std::to_string(connection_id) +
                                  " does not exist.");
  }
}

template <typename T>
message::Data Server<T>::handle_request(std::uint32_t connection_id, const message::Data& data) {
  utils::Logger::logger().info("Server::Handling request.");
  utils::Logger::logger().info("Server::Reveal connection id: " + std::to_string(connection_id) +
                               ".");
  auto connection = _connection_pool.get_connection(connection_id);
  if (connection) {
    return static_cast<T*>(this)->implement_handle_request(connection_id, data);
  } else {
    utils::Logger::logger().error("Server::Connection id: " + std::to_string(connection_id) +
                                  " does not exist.");
    return message::Data();
  }
}

} // namespace web_server

#endif // SERVER_H_
