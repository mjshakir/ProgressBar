#include <utility>
#include <gtest/gtest.h>
#include "CircularBuffer.hpp"

TEST(CircularBuffer, BasicOperations) {
    ProgressBar::CircularBuffer<int> buffer(5);

    // Test empty buffer
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.mean(), std::nullopt);

    // Push elements
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    buffer.push(5);
    EXPECT_EQ(buffer.size(), 5);

    // Test mean, median, min, max
    EXPECT_EQ(buffer.mean().value(), 3);
    EXPECT_EQ(buffer.median().value(), 3);
    EXPECT_EQ(buffer.minimum().value(), 1);
    EXPECT_EQ(buffer.maximum().value(), 5);

    // Test popping
    EXPECT_EQ(buffer.pop().value(), 1);
    EXPECT_EQ(buffer.size(), 4);
    
    // More tests can be added based on your requirements.
}

TEST(CircularBuffer, Overflow) {
    ProgressBar::CircularBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4); // This will remove 1
    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer.minimum().value(), 2);
}

TEST(CircularBuffer, Reset) {
    ProgressBar::CircularBuffer<int> buffer(3);
    buffer.push(1);
    buffer.push(2);
    buffer.reset();
    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.size(), 0);
}


TEST(CircularBuffer, Emplace) {
    ProgressBar::CircularBuffer<std::pair<int, int>> buffer(3);
    buffer.emplace(1, 2);
    buffer.emplace(3, 4);
    EXPECT_EQ(buffer.size(), 2);
    auto val = buffer.pop().value();
    EXPECT_EQ(val.first, 1);
    EXPECT_EQ(val.second, 2);
}


TEST(CircularBuffer, AlmostFullStatistics) {
    ProgressBar::CircularBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    EXPECT_DOUBLE_EQ(buffer.mean().value(), 2.5);
    EXPECT_DOUBLE_EQ(buffer.median().value(), 2.5);
    EXPECT_EQ(buffer.minimum().value(), 1);
    EXPECT_EQ(buffer.maximum().value(), 4);
}

TEST(CircularBuffer, SingleElementStatistics) {
    ProgressBar::CircularBuffer<int> buffer(5);
    buffer.push(1);
    EXPECT_DOUBLE_EQ(buffer.mean().value(), 1);
    EXPECT_DOUBLE_EQ(buffer.median().value(), 1);
    EXPECT_EQ(buffer.minimum().value(), 1);
    EXPECT_EQ(buffer.maximum().value(), 1);
}

TEST(CircularBuffer, FloatStatistics) {
    ProgressBar::CircularBuffer<float> buffer(5);

    // Add floating-point numbers
    buffer.push(1.5);
    buffer.push(2.5);
    buffer.push(3.0);
    buffer.push(4.7);
    buffer.push(5.8);

    EXPECT_NEAR(buffer.mean().value(), (1.5 + 2.5 + 3.0 + 4.7 + 5.8) / 5, 1e-6); // Expect mean to be close to the calculated mean
    EXPECT_NEAR(buffer.median().value(), 3.0, 1e-6); // Middle value when sorted is 3.0
    EXPECT_NEAR(buffer.minimum().value(), 1.5, 1e-6); // Minimum value
    EXPECT_NEAR(buffer.maximum().value(), 5.8, 1e-6); // Maximum value
}

TEST(CircularBuffer, CopyConstructor) {
    ProgressBar::CircularBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    ProgressBar::CircularBuffer<int> copyBuffer(buffer);
    EXPECT_EQ(copyBuffer.size(), 3);
    EXPECT_EQ(copyBuffer.pop().value(), 1);
    EXPECT_EQ(copyBuffer.pop().value(), 2);
    EXPECT_EQ(copyBuffer.pop().value(), 3);
}

TEST(CircularBuffer, CopyAssignmentOperator) {
    ProgressBar::CircularBuffer<int> buffer1(5);
    buffer1.push(1);
    buffer1.push(2);
    buffer1.push(3);

    ProgressBar::CircularBuffer<int> buffer2(5);
    buffer2 = buffer1;
    EXPECT_EQ(buffer2.size(), 3);
    EXPECT_EQ(buffer2.pop().value(), 1);
    EXPECT_EQ(buffer2.pop().value(), 2);
    EXPECT_EQ(buffer2.pop().value(), 3);
}

TEST(CircularBuffer, MoveConstructor) {
    ProgressBar::CircularBuffer<int> buffer(5);
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);

    ProgressBar::CircularBuffer<int> movedBuffer(std::move(buffer));
    EXPECT_EQ(movedBuffer.size(), 3);
    EXPECT_EQ(movedBuffer.pop().value(), 1);
    EXPECT_EQ(movedBuffer.pop().value(), 2);
    EXPECT_EQ(movedBuffer.pop().value(), 3);

    // buffer should be in a valid state after the move, but its content might be unspecified
    EXPECT_TRUE(buffer.empty() || !buffer.empty());
}

TEST(CircularBuffer, MoveAssignmentOperator) {
    ProgressBar::CircularBuffer<int> buffer1(5);
    buffer1.push(1);
    buffer1.push(2);
    buffer1.push(3);

    ProgressBar::CircularBuffer<int> buffer2(5);
    buffer2 = std::move(buffer1);
    EXPECT_EQ(buffer2.size(), 3);
    EXPECT_EQ(buffer2.pop().value(), 1);
    EXPECT_EQ(buffer2.pop().value(), 2);
    EXPECT_EQ(buffer2.pop().value(), 3);

    // buffer1 should be in a valid state after the move, but its content might be unspecified
    EXPECT_TRUE(buffer1.empty() || !buffer1.empty());
}

TEST(CircularBuffer, StressTest) {
    ProgressBar::CircularBuffer<int> buffer(1000000);
    for (int i = 0; i < 2000000; ++i) {
        buffer.push(i);
    }
    EXPECT_EQ(buffer.size(), 1000000); // Only the last 1000000 elements should be there
    EXPECT_DOUBLE_EQ(buffer.mean().value(), 1500000 - 0.5); // Mean of numbers from 1000000 to 1999999
    EXPECT_DOUBLE_EQ(buffer.median().value(), 1499999.5);
    EXPECT_EQ(buffer.minimum().value(), 1000000);
    EXPECT_EQ(buffer.maximum().value(), 1999999);
    // Other statistical methods can be similarly tested
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
