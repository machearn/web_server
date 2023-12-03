#include "../include/request.hpp"

#include <gtest/gtest.h>

class RequestTest: public ::testing::Test {
protected:
  void SetUp() override {
    std::string data_str =
        "POST / HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Content-Length: 12\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko)"
        " Chrome/80.0.3987.149 Safari/537.36\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,"
        "*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
        "Sec-Fetch-Site: none\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "\r\n"
        "Hello World!";
    data = web_server::message::Data(reinterpret_cast<const uint8_t*>(data_str.data()),
                                     data_str.size(), 0);
  }
  web_server::message::Data data;
};

TEST_F(RequestTest, Parse) {
  web_server::message::Request request(data);
  EXPECT_EQ(request.header().method(), web_server::message::Method::POST);
  EXPECT_EQ(request.header().path(), "/");
  EXPECT_EQ(request.header().version(), "HTTP/1.1");
  EXPECT_TRUE(request.header().contain("Content-Length"));
  EXPECT_EQ(request.header().get("Content-Length"), "12");
  EXPECT_EQ(request.body(), "Hello World!");
}

TEST_F(RequestTest, ToBytes) {
  web_server::message::Request request(data);
  std::string request_str{};
  request.to_bytes(request_str);
  EXPECT_EQ(data.size(), request_str.size());
}
