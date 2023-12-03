#include "../include/queue.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

class QueueTest: public testing::Test {
protected:
  web_server::utils::Queue<int> queue;
};

TEST_F(QueueTest, PushAndPop) {
  queue.push(1);
  queue.push(2);
  queue.push(3);
  EXPECT_EQ(queue.pop(), 1);
  EXPECT_EQ(queue.pop(), 2);
  EXPECT_EQ(queue.pop(), 3);
}

TEST_F(QueueTest, Empty) {
  EXPECT_TRUE(queue.empty());
  queue.push(1);
  EXPECT_FALSE(queue.empty());
  queue.pop();
  EXPECT_TRUE(queue.empty());
}

TEST_F(QueueTest, Size) {
  EXPECT_EQ(queue.size(), 0);
  queue.push(1);
  EXPECT_EQ(queue.size(), 1);
  queue.push(2);
  EXPECT_EQ(queue.size(), 2);
  queue.push(3);
  EXPECT_EQ(queue.size(), 3);
  queue.pop();
  EXPECT_EQ(queue.size(), 2);
  queue.pop();
  EXPECT_EQ(queue.size(), 1);
  queue.pop();
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(QueueTest, MoveConstructor) {
  queue.push(1);
  queue.push(2);
  queue.push(3);
  web_server::utils::Queue<int> other_queue{std::move(queue)};
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(other_queue.size(), 3);
  EXPECT_EQ(other_queue.pop(), 1);
  EXPECT_EQ(other_queue.pop(), 2);
  EXPECT_EQ(other_queue.pop(), 3);
}

TEST_F(QueueTest, MoveAssignment) {
  queue.push(1);
  queue.push(2);
  queue.push(3);
  web_server::utils::Queue<int> other_queue;
  EXPECT_TRUE(other_queue.empty());
  other_queue = std::move(queue);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(other_queue.size(), 3);
  EXPECT_EQ(other_queue.pop(), 1);
  EXPECT_EQ(other_queue.pop(), 2);
  EXPECT_EQ(other_queue.pop(), 3);
}

TEST_F(QueueTest, MultithreadPushAndPop) {
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; i++) {
    threads.emplace_back([this]() {
      for (int j = 0; j < 100; j++) {
        queue.push(j);
      }
    });
  }

  for (int i = 0; i < 10; i++) {
    threads.emplace_back([this]() {
      for (int j = 0; j < 100; j++) {
        queue.pop();
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_TRUE(queue.empty());
}

TEST_F(QueueTest, TwoThreadsPushAndPop) {
  std::thread pop_thread{[this]() {
    for (int i = 0; i < 1000; i++) {
      queue.pop();
    }
  }};

  for (int i = 0; i < 1000; i++) {
    queue.push(i);
  }

  pop_thread.join();

  EXPECT_TRUE(queue.empty());
}
