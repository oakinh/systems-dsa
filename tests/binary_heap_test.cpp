#include <gtest/gtest.h>
#include <systems_dsa/binary_heap.hpp>

TEST(BinaryHeapTest, DefaultInitializationYieldsSizeZero) {
    systems_dsa::binary_heap<int> heap {};
    EXPECT_EQ(heap.size(), 0);
}

TEST(BinaryHeapTest, PushPreservesOrderProperty) {
    systems_dsa::binary_heap<int, std::less<>> heap {};
    heap.push(10);
    heap.push(2);
    heap.push(55);
    heap.push(33);
    heap.push(12);
    heap.push(1000);
}
