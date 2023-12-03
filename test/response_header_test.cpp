#include "../include/response_header.hpp"

#include <gtest/gtest.h>

class ResponseHeaderTest: public ::testing::Test {
protected:
  void SetUp() override {
    std::string header_str = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: 0\r\n"
                             "\r\n";
    data.append(reinterpret_cast<const std::uint8_t*>(header_str.data()), header_str.size());
  }
  web_server::message::Data data{};
};

TEST_F(ResponseHeaderTest, Parse) {
  web_server::message::ResponseHeader header{};
  ASSERT_TRUE(header.parse(data));
  ASSERT_EQ(header.version(), "HTTP/1.1");
  ASSERT_EQ(header.status_code(), 200);
  ASSERT_EQ(header.status_message(), "OK");

  ASSERT_TRUE(header.contain("Content-Type"));
  ASSERT_EQ(header.get("Content-Type"), "text/html");
  ASSERT_TRUE(header.contain("Content-Length"));
  ASSERT_EQ(header.get("Content-Length"), "0");
}

TEST_F(ResponseHeaderTest, ToBytes) {
  web_server::message::ResponseHeader header{};
  ASSERT_TRUE(header.parse(data));
  std::string bytes{};
  ASSERT_EQ(header.to_bytes(bytes), data.size());
}
