#include "../include/connection_pool.hpp"
#include "include/mock_socket.hpp"

#include <gtest/gtest.h>

using MockConnection = web_server::connection::Connection<MockAsioSocket>;
using MockConnectionPool = web_server::connection::ConnectionPool<MockAsioSocket>;

TEST(ConnectionPoolTest, DefaultConstructor) {
  MockConnectionPool pool{};
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.max_size(), std::numeric_limits<std::uint16_t>::max());
  EXPECT_TRUE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
}

TEST(ConnectionPoolTest, Constructor) {
  MockConnectionPool pool{10};
  EXPECT_EQ(pool.size(), 0);
  EXPECT_EQ(pool.max_size(), 10);
  EXPECT_TRUE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
}

TEST(ConnectionPoolTest, Add) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{10};
  MockAsioSocket socket{io_context, ""};
  auto connection = std::make_shared<MockConnection>(io_context, std::move(socket), queue);
  auto id = pool.add(connection);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);
}

TEST(ConnectionPoolTest, Emplace) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{10};
  MockAsioSocket socket{io_context, ""};
  auto id = pool.emplace(io_context, std::move(socket), queue);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);
}

TEST(ConnectionPoolTest, AddToFull) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{1};
  MockAsioSocket socket{io_context, ""};
  auto connection = std::make_shared<MockConnection>(io_context, std::move(socket), queue);
  auto id = pool.add(connection);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_TRUE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);

  MockAsioSocket socket2{io_context, ""};
  auto connection2 = std::make_shared<MockConnection>(io_context, std::move(socket2), queue);
  auto id2 = pool.add(connection2);
  EXPECT_EQ(id2, -1);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_TRUE(pool.is_full());
  EXPECT_EQ(pool.get_connection(id2), nullptr);
}

TEST(ConnectionPoolTest, Erase) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{10};
  MockAsioSocket socket{io_context, ""};
  auto connection = std::make_shared<MockConnection>(io_context, std::move(socket), queue);
  auto id = pool.add(connection);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);

  pool.erase(id);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_TRUE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_EQ(pool.get_connection(id), nullptr);
}

TEST(ConnectionPoolTest, EraseAll) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{10};
  MockAsioSocket socket{io_context, ""};
  auto connection = std::make_shared<MockConnection>(io_context, std::move(socket), queue);
  auto id = pool.add(connection);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);

  pool.erase_all();
  EXPECT_EQ(pool.size(), 0);
  EXPECT_TRUE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_EQ(pool.get_connection(id), nullptr);
}

TEST(ConnectionPoolTest, EraseUnavaliable) {
  boost::asio::io_context io_context{};
  web_server::utils::Queue<web_server::message::Data> queue{};
  MockConnectionPool pool{10};
  MockAsioSocket socket{io_context, ""};
  auto connection = std::make_shared<MockConnection>(io_context, std::move(socket), queue);
  auto id = pool.add(connection);
  EXPECT_GE(id, 0);
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);

  pool.erase_unavaliable();
  EXPECT_EQ(pool.size(), 1);
  EXPECT_FALSE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_NE(pool.get_connection(id), nullptr);

  auto ec = connection->finish();
  pool.erase_unavaliable();
  EXPECT_FALSE(ec);
  EXPECT_EQ(pool.size(), 0);
  EXPECT_TRUE(pool.is_empty());
  EXPECT_FALSE(pool.is_full());
  EXPECT_EQ(pool.get_connection(id), nullptr);
}
