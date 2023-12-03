#include "../include/connection.hpp"
#include "include/mock_socket.hpp"

#include <gtest/gtest.h>
#include <memory>

class ConnectionTest : public ::testing::Test {
protected:
  ConnectionTest() : m_io_context(), m_socket(m_io_context, m_read), m_queue() {
    m_connection =
        std::make_shared<web_server::connection::Connection<MockAsioSocket>>(
            m_io_context, std::move(m_socket), m_queue);
  }
  void SetUp() override { m_io_context.run(); }
  void TearDown() override { m_io_context.stop(); }

  std::string m_read =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: "
      "11\r\n\r\nHello World";
  boost::asio::io_context m_io_context;
  MockAsioSocket m_socket;
  web_server::utils::Queue<web_server::message::Data> m_queue;
  std::shared_ptr<web_server::connection::Connection<MockAsioSocket>>
      m_connection;
};

TEST_F(ConnectionTest, Receive) {
  m_connection->receive(0);
  auto data = m_queue.pop();
  EXPECT_EQ(data.connection_id(), 0);
  EXPECT_EQ(data.size(), m_read.size());
  EXPECT_EQ(std::string((char *)data.data(), data.size()), m_read);
}

TEST_F(ConnectionTest, Send) {
  web_server::message::Data data(
      reinterpret_cast<const std::uint8_t *>(m_read.data()), m_read.size(), 0);
  m_connection->send(data);
  EXPECT_EQ(m_connection->socket().get_write(), m_read);
}
