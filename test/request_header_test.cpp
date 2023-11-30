#include "../include/request_header.hpp"

#include <gtest/gtest.h>

class RequestHeaderTest : public ::testing::Test {
protected:
  void SetUp() override {
    data.append(reinterpret_cast<const std::uint8_t*>(header_str.data()), header_str.size());
  }
  web_server::message::Data data;
  std::string header_str = "GET / HTTP/1.1\r\n"
                  "Host: localhost:8080\r\n"
                  "Connection: keep-alive\r\n"
                  "Cache-Control: max-age=0\r\n"
                  "Upgrade-Insecure-Requests: 1\r\n"
                  "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                  "(KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36\r\n"
                  "Sec-Fetch-Dest: document\r\n"
                  "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,"
                  "image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
                  "Sec-Fetch-Site: none\r\n"
                  "Sec-Fetch-Mode: navigate\r\n"
                  "Sec-Fetch-User: ?1\r\n"
                  "Accept-Encoding: gzip, deflate, br\r\n"
                  "Accept-Language: en-US,en;q=0.9\r\n"
                  "\r\n";
};

TEST_F(RequestHeaderTest, Parse) {
  web_server::message::RequestHeader header;
  EXPECT_TRUE(header.parse(data));
  EXPECT_EQ(header.method(), web_server::message::Method::GET);
  EXPECT_EQ(header.path(), "/");
  EXPECT_EQ(header.version(), "HTTP/1.1");
  EXPECT_TRUE(header.contain("Host"));
  EXPECT_EQ(header.get("Host"), "localhost:8080");
  EXPECT_TRUE(header.contain("Connection"));
  EXPECT_EQ(header.get("Connection"), "keep-alive");
  EXPECT_TRUE(header.contain("Cache-Control"));
  EXPECT_EQ(header.get("Cache-Control"), "max-age=0");
  EXPECT_TRUE(header.contain("Accept-Encoding"));
  EXPECT_EQ(header.get("Accept-Encoding"), "gzip, deflate, br");
  EXPECT_TRUE(header.contain("Accept-Language"));
  EXPECT_EQ(header.get("Accept-Language"), "en-US,en;q=0.9");
}

TEST_F(RequestHeaderTest, ToBytes) {
  web_server::message::RequestHeader header;
  std::string bytes{};
  EXPECT_TRUE(header.parse(data));
  EXPECT_EQ(header.to_bytes(bytes), 0);
  EXPECT_EQ(bytes.size(), header_str.size());
}
