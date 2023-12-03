#include "../include/response.hpp"

#include <gtest/gtest.h>

class ResponseTest: public ::testing::Test {
protected:
  void SetUp() override {
    std::string response_str = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: 13\r\n"
                               "\r\n"
                               "Hello, World!";
    data = web_server::message::Data(reinterpret_cast<const std::uint8_t*>(response_str.data()),
                                     response_str.size(), 0);
  }

  web_server::message::Data data;
};

TEST_F(ResponseTest, Parse) {
  web_server::message::Response response(data);

  EXPECT_EQ(response.header().version(), "HTTP/1.1");
  EXPECT_EQ(response.header().status_code(), 200);
  EXPECT_EQ(response.header().status_message(), "OK");
  EXPECT_TRUE(response.header().contain("Content-Type"));
  EXPECT_EQ(response.header().get("Content-Type"), "text/html");
  EXPECT_TRUE(response.header().contain("Content-Length"));
  EXPECT_EQ(response.header().get("Content-Length"), "13");

  EXPECT_EQ(response.body(), "Hello, World!");
}

TEST_F(ResponseTest, ToBytes) {
  web_server::message::Response response(data);
  std::string bytes;
  response.to_bytes(bytes);

  EXPECT_EQ(bytes.size(), data.size());
}
