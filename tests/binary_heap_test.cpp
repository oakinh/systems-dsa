#include <gtest/gtest.h>
#include <systems_dsa/binary_heap.hpp>

TEST(BinaryHeapTest, DefaultInitializationYieldsSizeZero) {
    systems_dsa::binary_heap<int> heap {};
    EXPECT_EQ(heap.size(), 0);
}
