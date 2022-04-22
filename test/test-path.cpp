//
// Created by dingjing on 4/22/22.
//

#include <gtest/gtest.h>

TEST(TestPath1, BasicAssertions) {
    EXPECT_EQ(7 * 6, 42);
}

int main (int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}