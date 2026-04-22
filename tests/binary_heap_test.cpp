#include "utils/lifetime_tracker.hpp"

#include <gtest/gtest.h>
#include <systems_dsa/binary_heap.hpp>

class BinaryHeapTest_F : public testing::Test {
protected:
    systems_dsa::binary_heap<LifetimeTracker, std::less<>> heap {};

    BinaryHeapTest_F() {
        heap.push(10);
        heap.push(2);
        heap.push(55);
        heap.push(33);
        heap.push(12);
        heap.push(1000);
        heap.push(33);
        heap.push(12);
        heap.push(66);
        heap.push(79);
    }
};

template <typename T, typename Compare>
::testing::AssertionResult IsValidPopOrder(systems_dsa::binary_heap<T, Compare> heap) {
    Compare comp;
    std::vector<T> popped {};

    while (!heap.empty()) {
        popped.push_back(heap.top());
        heap.pop();
    }

    for (std::size_t i { 1 }; i < popped.size(); ++i) {
        // Initialize to 1 to avoid unsigned overflow
        const T& prev = popped[i - 1];
        const T& curr = popped[i];


        // Violated invariant: top() does NOT compare as before any other element in comp's strict weak ordering
        if (comp(prev, curr)) {
            return testing::AssertionFailure()
                << "heap pop order violation at index " << i
                << "previous element had lower priority than current element";
        }
    }

    return testing::AssertionSuccess();
}

///////////////////////////////
// Basic functionality tests //
///////////////////////////////

TEST(BinaryHeapTest, DefaultInitializationYieldsSizeZero) {
    systems_dsa::binary_heap<int> heap {};
    EXPECT_EQ(heap.size(), 0);
}

TEST(BinaryHeapTest, PushPreservesOrderProperty) {
    systems_dsa::binary_heap<int, std::less<>> heap {};

    std::vector nums {
        10,
        2,
        55,
        33,
        12,
        1000,
        33
        };

    for (const auto num : nums) {
        heap.push(num);
    }
    EXPECT_EQ(heap.size(), nums.size());

    EXPECT_TRUE(IsValidPopOrder(heap));
}

/////////////////////////
// Adversarial testing //
/////////////////////////
