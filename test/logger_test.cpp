#include "../include/logger.hpp"

#include <gtest/gtest.h>
#include <regex>
#include <string>
#include <thread>

TEST(LoggerTest, Log) {
  using namespace web_server::utils;
  std::stringstream ss{};

  Logger::logger(ss);
  Logger::logger().info("test");

  std::string pattern{R"(\[INFO\]: test)"};

  std::regex re{pattern};

  EXPECT_TRUE(std::regex_search(ss.str(), re));
}

TEST(LoggerTest, MultiThread) {
  using namespace web_server::utils;
  std::stringstream ss{};

  Logger::logger(ss);

  std::vector<std::thread> threads{};
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&]() { Logger::logger().info("test"); });
  }

  for (auto& t : threads) {
    t.join();
  }

  std::string pattern{R"(\[INFO\]: test)"};

  std::regex re{pattern};
  std::smatch match{};

  std::string line{};
  int count = 0;
  while (std::getline(ss, line)) {
    EXPECT_TRUE(std::regex_search(line, match, re));
    EXPECT_EQ(match.size(), 1);
    EXPECT_EQ(match.str(), "[INFO]: test");
    ++count;
  }
  EXPECT_EQ(count, 10);
}
