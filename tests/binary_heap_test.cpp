#include "utils/lifetime_tracker.hpp"
#include "utils/seed.hpp"

#include <gtest/gtest.h>
#include <systems_dsa/binary_heap.hpp>
#include <queue>
#include <functional>

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

TEST(BinaryHeapTest, EmptyReturnsExpectedBool) {
    systems_dsa::binary_heap<int> heap {};
    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0);
    EXPECT_GT(heap.capacity(), 0);
    heap.push(0);
    EXPECT_FALSE(heap.empty());
    EXPECT_EQ(heap.size(), 1);
}

TEST_F(BinaryHeapTest_F, TopReturnsHighestPriorityElement) {
    EXPECT_EQ(heap.top().id, 1000);
}

TEST(BinaryHeapTest, EmplaceConstructsInContainer) {
    systems_dsa::binary_heap<LifetimeTracker> heap {};
    LifetimeTracker::resetCounts();
    heap.emplace(1);
    EXPECT_EQ(LifetimeTracker::copyCtorCount, 0);
    EXPECT_EQ(LifetimeTracker::moveCtorCount, 0);
    EXPECT_EQ(heap.size(), 1);
    LifetimeTracker::resetCounts();
}

TEST(BinaryHeapTest, PushedLValueCopyConstructsOnce) {
    systems_dsa::binary_heap<LifetimeTracker> heap {};
    LifetimeTracker myTracker { 15 };
    LifetimeTracker::resetCounts();
    heap.push(myTracker);
    EXPECT_EQ(LifetimeTracker::copyCtorCount, 1);
    EXPECT_EQ(LifetimeTracker::moveCtorCount, 0);
    LifetimeTracker::resetCounts();
}

TEST(BinaryHeapTest, PushedRValueMoveConstructsOnce) {
    systems_dsa::binary_heap<LifetimeTracker> heap {};
    LifetimeTracker::resetCounts();
    heap.push(LifetimeTracker{15});

    EXPECT_EQ(LifetimeTracker::moveCtorCount, 1);
    EXPECT_EQ(LifetimeTracker::copyCtorCount, 0);

    LifetimeTracker::resetCounts();
}

TEST(BinaryHeapTest, ReserveAllowsNElementsWithoutReallocation) {
    systems_dsa::binary_heap<LifetimeTracker> heap {};
    ASSERT_LT(heap.capacity(), 100);
    heap.reserve(100);
    std::size_t capacity { heap.capacity() };
    for (std::size_t i {}; i < 100; ++i) {
        heap.push({}); // Should not cause a reallocation
    }
    ASSERT_EQ(heap.capacity(), capacity);
    LifetimeTracker::resetCounts();
    heap.push({}); // Should cause a reallocation
    ASSERT_GT(heap.capacity(), capacity);
    ASSERT_GE(LifetimeTracker::moveCtorCount, 100);
}

/////////////////////////
// Adversarial testing //
/////////////////////////

TEST(BinaryHeapTest, RandomSeqPushPop) {
    std::uint64_t seed { getSeed("BHEAP_SEED") };
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<> valDist (1, 1000);

    std::priority_queue<LifetimeTracker, std::vector<LifetimeTracker>, std::less<>> reference;
    systems_dsa::binary_heap<LifetimeTracker, std::less<>> heap;

    for (std::size_t i {}; i < 1000; ++i) {
        int val { valDist(rng) };
        heap.push({ val });
        reference.push({ val });
    }
    EXPECT_TRUE(IsValidPopOrder(heap));
    ASSERT_EQ(reference.size(), heap.size());
    ASSERT_FALSE(reference.empty() && heap.empty());
    ASSERT_EQ(reference.top(), heap.top());

    enum class OP : uint8_t {
        PUSH,
        POP
    };

    std::uniform_int_distribution<> opDist(0, 1);

    for (std::size_t i {}; i < 10'000; ++i) {
        OP op { static_cast<OP>(opDist(rng)) };
        if (heap.empty() && reference.empty()) op = OP::PUSH;
        switch (op) {
        case OP::PUSH: {
            int val { valDist(rng) };
            heap.push({ val });
            reference.push({ val });
            ASSERT_EQ(heap.size(), reference.size());
            ASSERT_FALSE(heap.empty() && reference.empty());
            ASSERT_EQ(heap.top(), reference.top());
            break;
        }
        case OP::POP:
            ASSERT_FALSE(heap.empty() && reference.empty());
            ASSERT_EQ(heap.top(), reference.top());
            heap.pop();
            reference.pop();
            ASSERT_EQ(heap.size(), reference.size());
            break;
        }
    }
    EXPECT_TRUE(IsValidPopOrder(heap));
}