#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <boost/asio.hpp>
#include <functional>

class MockAsioSocket: public boost::asio::ip::tcp::socket {
public:
  MockAsioSocket(boost::asio::io_context& io_context, const std::string& read)
      : boost::asio::ip::tcp::socket(io_context), m_read(read) {}
  MockAsioSocket(const MockAsioSocket&) = delete;
  MockAsioSocket(MockAsioSocket&&) = default;

  void async_write_some(const boost::asio::const_buffers_1& buffer,
                        std::function<void(boost::system::error_code, std::size_t)> callback);

  void async_read_some(const boost::asio::mutable_buffers_1& buffer,
                       std::function<void(boost::system::error_code, std::size_t)> callback);

  const std::string& get_write() const { return m_write; }
  std::string& get_write() { return m_write; }

  bool is_open() const { return m_is_open; }
  void shutdown(boost::asio::ip::tcp::socket::shutdown_type type) { m_is_open = false; }
  void shutdown(boost::asio::ip::tcp::socket::shutdown_type type,
                boost::system::error_code& error) {
    m_is_open = false;
  }
  void close() { m_is_open = false; }
  void close(boost::system::error_code& error) { m_is_open = false; }

private:
  bool m_is_open = true;
  bool m_is_read = false;
  std::string m_read;
  std::string m_write{};
};

#endif // MOCK_SOCKET_H
