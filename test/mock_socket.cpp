#include "include/mock_socket.hpp"

void MockAsioSocket::async_write_some(
    const boost::asio::const_buffers_1& buffer,
    std::function<void(boost::system::error_code, std::size_t)> callback) {
  m_write = std::string(boost::asio::buffers_begin(buffer), boost::asio::buffers_end(buffer));
  callback(boost::system::error_code{}, m_write.size());
}

void MockAsioSocket::async_read_some(
    const boost::asio::mutable_buffers_1& buffer,
    std::function<void(boost::system::error_code, std::size_t)> callback) {
  if (!m_is_read) {
    m_is_read = true;
    std::copy(m_read.begin(), m_read.end(), boost::asio::buffers_begin(buffer));
    callback(boost::system::error_code{}, m_read.size());
  } else {
    callback(boost::asio::error::eof, 0);
  }
}
