#include "../include/data.hpp"

#include <gtest/gtest.h>

class DataTest: public testing::Test {
protected:
  web_server::message::Data data{};
};

TEST_F(DataTest, Capacity) { EXPECT_EQ(data.capacity(), 0); }

TEST_F(DataTest, Empty) { EXPECT_EQ(data.size(), 0); }

TEST_F(DataTest, NullData) { EXPECT_EQ(data.data(), nullptr); }

TEST_F(DataTest, SetGetConnectionId) {
  data.set_connection_id(123);

  EXPECT_EQ(data.connection_id(), 123);
}

TEST_F(DataTest, EmptyReserve) {
  data.reserve(256);

  EXPECT_NE(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 256);
  EXPECT_EQ(data.size(), 0);
}

TEST_F(DataTest, AppendEnoughCapacity) {
  data.reserve(256);
  data.append(reinterpret_cast<const std::uint8_t*>("Hello"), 5);

  EXPECT_NE(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 256);
  EXPECT_EQ(data.size(), 5);
  EXPECT_EQ(std::memcmp(data.data(), "Hello", 5), 0);
}

TEST_F(DataTest, Move) {
  data.reserve(256);
  data.append(reinterpret_cast<const std::uint8_t*>("Hello"), 5);
  web_server::message::Data data2(std::move(data));

  EXPECT_EQ(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 0);
  EXPECT_EQ(data.size(), 0);
  EXPECT_NE(data2.data(), nullptr);
  EXPECT_EQ(data2.capacity(), 256);
  EXPECT_EQ(data2.size(), 5);
  EXPECT_EQ(std::memcmp(data2.data(), "Hello", 5), 0);

  web_server::message::Data data3{};
  data3 = std::move(data2);

  EXPECT_EQ(data2.data(), nullptr);
  EXPECT_EQ(data2.capacity(), 0);
  EXPECT_EQ(data2.size(), 0);
  EXPECT_NE(data3.data(), nullptr);
  EXPECT_EQ(data3.capacity(), 256);
  EXPECT_EQ(data3.size(), 5);
  EXPECT_EQ(std::memcmp(data3.data(), "Hello", 5), 0);
}

TEST_F(DataTest, AppendNotEnoughCapacity) {
  data.append(reinterpret_cast<const std::uint8_t*>("Hello"), 5);

  EXPECT_NE(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 5);
  EXPECT_EQ(data.size(), 5);
  EXPECT_EQ(std::memcmp(data.data(), "Hello", 5), 0);
}

TEST_F(DataTest, AppendFromStream) {
  data.append(reinterpret_cast<const std::uint8_t*>("Hello"), 5);
  std::stringstream ss;
  ss << "World";
  data.append(ss, 5);

  EXPECT_NE(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 10);
  EXPECT_EQ(data.size(), 10);
  EXPECT_EQ(std::memcmp(data.data(), "HelloWorld", 10), 0);
}

TEST_F(DataTest, Clear) {
  data.clear();

  EXPECT_EQ(data.data(), nullptr);
  EXPECT_EQ(data.capacity(), 0);
  EXPECT_EQ(data.size(), 0);
}
