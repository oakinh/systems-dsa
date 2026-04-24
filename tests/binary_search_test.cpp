#include <gtest/gtest.h>
#include <cstddef>
#include <functional>
#include <systems_dsa/binary_search.hpp>

/////////////////
// lower_bound //
/////////////////
TEST(LowerBoundTest, FindsLowerBoundWhenEquivalent) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 8, std::less<>{}),
        7); // Index of val 8
}

TEST(BinarySearchTest, FindsLowerBoundWhenNotEquivalent) {
    std::array arr { 1, 2, 3, 4, 5, 6, 7, 9, 10 }; // 8 isn't present
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 8, std::less<>{}),
        7); // Index of val 9
}

TEST(LowerBoundTest, ReturnsEndWhenAllOrderBefore) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 11, std::less<>{}),
        arr.size()
        );
}

TEST(LowerBoundTest, CorrectlyReturnsIndexZeroWhenNotEquivalent) {
    // Target not present in arr
    std::array arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 0, std::less<>{}),
        0);
}

TEST(LowerBoundTest, CorrectlyReturnsIndexZeroWhenIsEquivalent) {
    // Target is present in arr
    std::array arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 1, std::less<>{}),
        0);
}

TEST(LowerBoundTest, CorrectlyReturnsLastInBoundsIndexWhenNotEquivalent) {
    // Target is not present in arr
    std::array arr { 1, 2, 3, 4, 5, 6, 7, 8, 9 , 11 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 10, std::less<>{}),
        arr.size() - 1);
}

TEST(LowerBoundTest, CorrectlyReturnsLastInBoundsIndexWhenIsEquivalent) {
    // Target is present in arr
    std::array arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 10, std::less<>{}),
        arr.size() - 1);
}

TEST(LowerBoundTest, ReturnsFirstNotOrderedBeforeWithDuplicatesPresent) {
    std::array arr { 1, 2, 3, 3, 3, 3, 4, 5, 6, 7 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 3, std::less<>{}),
        2);
}

///////////////////
// binary_search //
///////////////////

TEST(BinarySearchTest, BinarySearchReturnsTrueWhenPresent) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_TRUE(systems_dsa::binary_search(arr.data(), arr.size(), 8, std::less<>{}));
}

TEST(BinarySearchTest, BinarySearchReturnsFalseWhenNotPresent) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 9, 10 }; // No 8 in arr
    EXPECT_FALSE(systems_dsa::binary_search(arr.data(), arr.size(), 8, std::less<>{}));
}
