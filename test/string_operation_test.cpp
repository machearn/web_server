#include "../include/utils.hpp"

#include <gtest/gtest.h>
#include <string>

TEST(StringOperationTest, GetLine) {
  std::string data = "GET / HTTP/1.1\r\n"
                     "Content-Length: 0\r\n\r\n";

  std::string_view line;
  int start = 0;

  int ret = web_server::utils::get_line(data, line, &start);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(line.compare("GET / HTTP/1.1"), 0);
  ASSERT_EQ(start, 16);

  ret = web_server::utils::get_line(data, line, &start);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(line.compare("Content-Length: 0"), 0);
  ASSERT_EQ(start, 35);

  ret = web_server::utils::get_line(data, line, &start);
  ASSERT_EQ(ret, -1);
}

TEST(StringOperationTest, SplitLine) {
  std::string_view line = "GET / HTTP/1.1";
  std::vector<std::string_view> words;
  web_server::utils::split_line(line, words);

  ASSERT_EQ(words.size(), 3);
  ASSERT_EQ(words[0].compare("GET"), 0);
  ASSERT_EQ(words[1].compare("/"), 0);
  ASSERT_EQ(words[2].compare("HTTP/1.1"), 0);
}

TEST(StringOperationTest, SplitHead) {
  std::string_view line = "Content-Length: 0";
  std::string_view key, value;
  web_server::utils::split_head(line, key, value);

  ASSERT_EQ(key.compare("Content-Length"), 0);
  ASSERT_EQ(value.compare("0"), 0);
}
