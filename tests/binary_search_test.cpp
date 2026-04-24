#include <gtest/gtest.h>
#include <cstddef>
#include <functional>
#include <systems_dsa/binary_search.hpp>

TEST(BinarySearchTest, FindsLowerBound) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    std::size_t lowerIndex { systems_dsa::lower_bound(arr.data(), arr.size(), 8, std::less<>{}) };
    EXPECT_EQ(lowerIndex, 7);
}

TEST(BinarySearchTest, BinarySearchReturnsTrueWhenPresent) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_TRUE(systems_dsa::binary_search(arr.data(), arr.size(), 8, std::less<>{}));
}

TEST(BinarySearchTest, BinarySearchReturnsFalseWhenNotPresent) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 9, 10 }; // No 8 in arr
    EXPECT_FALSE(systems_dsa::binary_search(arr.data(), arr.size(), 8, std::less<>{}));
}

TEST(BinarySearchTest, LowerBoundReturnsEndWhenNoneOrderBefore) {
    std::array<int, 10> arr { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(systems_dsa::lower_bound(arr.data(), arr.size(), 0, std::less<>{}),
        arr.size()
        );
}
