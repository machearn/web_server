#include "../include/data_view.hpp"

#include <gtest/gtest.h>

TEST(DataViewTest, Constructor) {
  const std::uint8_t* data = nullptr;
  web_server::message::DataView data_view{data, 0, 0};

  EXPECT_EQ(data_view.data(), data);
  EXPECT_EQ(data_view.size(), 0);
  EXPECT_EQ(data_view.connection_id(), 0);

  web_server::message::Data data2{};
  web_server::message::DataView data_view2{data2};

  EXPECT_EQ(data_view2.data(), data2.data());
  EXPECT_EQ(data_view2.size(), data2.size());
  EXPECT_EQ(data_view2.connection_id(), data2.connection_id());
}

TEST(DataViewTest, SubData) {
  std::uint8_t* data = std::allocator<std::uint8_t>{}.allocate(10);
  for (std::uint8_t i = 0; i < 10; ++i) {
    data[i] = i;
  }
  web_server::message::DataView data_view{data, 10, 0};

  auto sub1 = data_view.subdata(0, 5);
  
  EXPECT_EQ(std::memcmp(sub1.data(), data_view.data(), 5), 0);
  EXPECT_EQ(sub1.size(), 5);

  auto sub2 = data_view.subdata(5);

  EXPECT_EQ(std::memcmp(sub2.data(), data_view.data() + 5, 5), 0);
  EXPECT_EQ(sub2.size(), 5);

  auto sub3 = data_view.subdata(5, 10);

  EXPECT_EQ(std::memcmp(sub3.data(), data_view.data() + 5, 5), 0);
  EXPECT_EQ(sub3.size(), 5);
}
