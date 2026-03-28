#include <gtest/gtest.h>
#include <systems_dsa/hash_map.hpp>
#include "utils/lifetime_tracker.hpp"
#include "utils/seed.hpp"

class HashMapTest_F : public testing::Test {
protected:
    systems_dsa::hash_map<int, std::size_t> hashMap {};
    std::vector<std::pair<int, std::size_t>> pairs {
                { 1, 101 },
                { 3, 103 },
                { 7, 107 },
                { 10, 110 },
                { 2, 102 },
            };
    HashMapTest_F() {
        for (const auto& pair : pairs) {
            hashMap.insert(pair);
        }
    }
};

TEST(HashMapTest, IsEmptyInitially) {
    systems_dsa::hash_map<std::string, int> hashMap {};
    EXPECT_EQ(hashMap.size(), 0);
}

TEST(HashMapTest, SizeConstructs) {
    systems_dsa::hash_map<int, int> hashMap { 10 };
    EXPECT_NO_FATAL_FAILURE();
}

TEST(HashMapTest, InsertAcceptsEither1Or2Args) {
    systems_dsa::hash_map<int, int> hashMap {};
    hashMap.insert(10, 1001);
    hashMap.insert({23, 100});
    EXPECT_EQ(hashMap.size(), 2) << "hashMap size should equal the number of pairs inserted";
    EXPECT_NO_FATAL_FAILURE();
}

TEST_F(HashMapTest_F, FindReturnsCorrectValue) {
    for (const auto& pair : pairs) {
        const auto& valPtr { hashMap.find(pair.first) };
        if (valPtr == nullptr) FAIL() << "valPtr was a nullptr, first: " << pair.first;
        EXPECT_EQ(*hashMap.find(pair.first), pair.second);
    }
}

TEST_F(HashMapTest_F, ContainsReturnsCorrectBool) {
    EXPECT_EQ(hashMap.size(), pairs.size());

    for (const auto& pair : pairs) {
        EXPECT_EQ(hashMap.contains(pair.first), true);
    }

    std::vector<int> nonKeyInts { 4, 5, 100, 22000, 101, 103, 107, 110, 102 };
    for (const auto& num : nonKeyInts) {
        EXPECT_EQ(hashMap.contains(num), false);
    }
}

TEST(HashMapTest, EmptyReturnsCorrectBool) {
    systems_dsa::hash_map<int, int> hashMap {};
    EXPECT_EQ(hashMap.empty(), true);
    hashMap.insert(10, 10);
    EXPECT_EQ(hashMap.empty(), false);
}

TEST(HashMapTest, EraseDestroysElement) {
    systems_dsa::hash_map<int, LifetimeTracker> hashMap {};
    LifetimeTracker::resetCounts();
    hashMap.insert(10, {});
    hashMap.insert(11, {});
    hashMap.insert(12, {});
    const int oldDtorCount { LifetimeTracker::dtorCount };
    EXPECT_EQ(hashMap.erase(10), 1);
    EXPECT_EQ(LifetimeTracker::dtorCount, oldDtorCount + 1);
    EXPECT_FALSE(hashMap.contains(10));
    EXPECT_EQ(hashMap.size(), 2) << "hashMap size should be one less after erase";
    LifetimeTracker::resetCounts();
}

TEST(HashMapTest, EraseReturnsZeroOnNonExistentKey) {
    systems_dsa::hash_map<int, LifetimeTracker> hashMap{};
    LifetimeTracker::resetCounts();
    hashMap.insert(10, {});
    hashMap.insert(11, {});
    hashMap.insert(12, {});
    const int oldDtorCount { LifetimeTracker::dtorCount };
    EXPECT_EQ(hashMap.erase(20), 0);
    EXPECT_EQ(LifetimeTracker::dtorCount, oldDtorCount);
    EXPECT_FALSE(hashMap.contains(20));
}

TEST_F(HashMapTest_F, RehashLosesNoElements) {
    for (const auto& p : pairs) {
        const auto* valPtr { hashMap.find(p.first) };
        if (valPtr == nullptr) FAIL() << "valPtr was a null ptr, first: " << p.first;
        EXPECT_EQ(*hashMap.find(p.first), p.second);
    }
}

TEST_F(HashMapTest_F, ClearRemovesAllElements) {
    hashMap.clear();
    EXPECT_EQ(hashMap.size(), 0);
    for (const auto& p : pairs) {
        EXPECT_FALSE(hashMap.contains(p.first));
    }
}

TEST_F(HashMapTest_F, ElementsIntactPostReserve) {
    std::size_t hashMapSize { hashMap.size() };
    hashMap.reserve(30);
    EXPECT_EQ(hashMap.size(), hashMapSize);
}

TEST(HashMapTest, RandomSeqInsertEraseContains) {
    std::uint64_t seed { getSeed("HASHMAP_SEED") };
    enum class OP {
        INSERT,
        ERASE,
        CONTAINS,
    };
}
