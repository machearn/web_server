#include "../include/data_buffer.hpp"

#include <gtest/gtest.h>

TEST(DataBufferTest, BasicFunctionality) {
  char data[]{"Hello\nWorld!"};
  web_server::message::DataBuffer data_buffer{data, 12};
  std::istream is{&data_buffer};
  std::string line;

  EXPECT_TRUE(std::getline(is, line));
  EXPECT_EQ(line, "Hello");

  EXPECT_TRUE(std::getline(is, line));
  EXPECT_EQ(line, "World!");

  EXPECT_FALSE(std::getline(is, line));
}
