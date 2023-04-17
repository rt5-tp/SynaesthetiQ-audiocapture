#include "../src/AudioCapture/PingPongBuffer.h"
#include <gtest/gtest.h>

// Tests empty buffer is not full.
TEST(isFullTest, ReturnsFalseWhenNotFull){
    PingPongBuffer buffer(10);
    EXPECT_EQ(buffer.is_full(), false);
}

// Tests full buffer is full.
TEST(isFullTest, ReturnsTrueWhenFull){
    PingPongBuffer buffer(2);
    std::vector<short> test_data{0,0,0};
    buffer.add_data(test_data);
    EXPECT_EQ(buffer.is_full(), !true);
}

// Tests buffer remains correct size.
TEST(getCurrentBufferTest, ReturnsCorrectSize){
    PingPongBuffer buffer(10);
    std::vector<short> test_data{0,0,0,0,0,0,0,0,0,0};
    buffer.add_data(test_data);
    EXPECT_EQ(buffer.get_current_buffer().size(), test_data.size()/2);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
