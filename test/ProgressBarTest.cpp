#include "gtest/gtest.h"
#include "ProgressBar.hpp"


// class ProgressBarTest : public ::testing::Test {
// protected:
//     ProgressBarTest() {
//         // You can do set-up work for each test here.
//     }

//     ~ProgressBarTest() override {
//         // You can do clean-up work that doesn't throw exceptions here.
//     }
// };

// // Test the constructor that only takes a name and the default characters.
// TEST_F(ProgressBarTest, NameConstructor) {
//     ProgressBar::ProgressBar pb("Test", "*", "-");
//     EXPECT_FALSE(pb.done());
// }

// // Test the constructor that takes total, name, and custom characters.
// TEST_F(ProgressBarTest, TotalNameConstructor) {
//     ProgressBar::ProgressBar pb(50, "Test", "*", "-");
//     EXPECT_FALSE(pb.done());
// }

// // Test the copy constructor.
// TEST_F(ProgressBarTest, CopyConstructor) {
//     ProgressBar::ProgressBar pb1(100, "CopyTest", "=", " ");
//     ProgressBar::ProgressBar pb2(pb1);
//     while (!pb2.done()){
//         pb2.update();
//     }
// }

// // Test the move constructor.
// TEST_F(ProgressBarTest, MoveConstructor) {
//     ProgressBar::ProgressBar pb1(100, "MoveTest", "=", " ");
//     ProgressBar::ProgressBar pb2(std::move(pb1));
//     while (!pb2.done()){
//         pb2.update();
//     }
// }

// // Test the update function without a total set.
// TEST_F(ProgressBarTest, UpdateIndefinite) {
//     ProgressBar::ProgressBar pb("Indefinite", "*", "-");
//     constexpr uint8_t target = 10;
//     uint8_t start = 0;
//     while (start < target){
//         ++start;
//         pb.update();
//     }
// }

// // Test the update function with a total set.
// TEST_F(ProgressBarTest, UpdateWithTotal) {
//     ProgressBar::ProgressBar pb(10, "UpdateTest", "*", "-");
//     for (int i = 0; i < 10; ++i) {
//         pb.update();
//     }
//     EXPECT_TRUE(pb.done());
// }

// // Test the done function.
// TEST_F(ProgressBarTest, DoneFunction) {
//     ProgressBar::ProgressBar pb(5, "DoneTest", "*", "-");
//     EXPECT_FALSE(pb.done());
//     for (int i = 0; i < 5; ++i) {
//         pb.update();
//     }
//     EXPECT_TRUE(pb.done());
// }

// // Test the ProgressBar when updating beyond the total.
// TEST_F(ProgressBarTest, UpdateBeyondTotal) {
//     ProgressBar::ProgressBar pb(1, "OverUpdateTest", "*", "-");
//     pb.update();
//     EXPECT_TRUE(pb.done());
//     pb.update();  // This update should not change the state.
//     EXPECT_TRUE(pb.done());
//     // You would need to check if the internal progress remains at the total.
// }


// // Main function for the test.
// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
