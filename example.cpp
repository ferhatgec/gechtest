#include "include/gechtest.hpp"

TEST(TEST_CASE) {
    ASSERT_EQ("gech", "gech")
    ASSERT_EQ("savort", "gech")
    ASSERT_UNEQ("poljg", "poljg")

    ASSERT_GT(10, 20)
    ASSERT_LT(20, 10)
    ASSERT_GEQ(20, 20)
}

TEST_MAIN