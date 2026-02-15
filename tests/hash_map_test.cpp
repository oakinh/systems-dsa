#include <gtest/gtest.h>
#include <systems_dsa/hash_map.hpp>

TEST(HashMapTest, DefaultConstructs) {
    systems_dsa::hash_map<std::string, int> hashMap {};
    EXPECT_GT(hashMap.size(), 0);
}

TEST(HashMapTest, InsertAcceptsEither1Or2Args) {
    systems_dsa::hash_map<int, int> hashMap {};
    hashMap.insert(10, 1001);
    hashMap.insert({23, 100});
}

TEST(HashMapTest, FindReturnsCorrectValue) {
    systems_dsa::hash_map<int, std::size_t> hashMap {};
    std::vector<std::pair<int, std::size_t>> pairsToInsert {
        { 1, 101 },
        { 3, 103 },
        { 7, 107 },
        { 10, 110 },
        { 2, 102 },
    };
    for (const auto& pair : pairsToInsert) {
        hashMap.insert(pair);
    }

    for (const auto& pair : pairsToInsert) {
        EXPECT_EQ(hashMap.find(pair.first), pair.second);
    }
}

TEST(HashMapTest, ContainsReturnsCorrectBool) {
    systems_dsa::hash_map<int, int> hashMap {};
    std::vector<std::pair<int, int>> pairsToInsert {
            { 1, 101 },
            { 3, 103 },
            { 7, 107 },
            { 10, 110 },
            { 2, 102 },
        };
    for (const auto& pair : pairsToInsert) {
        hashMap.insert(pair);
    }

    for (const auto& pair : pairsToInsert) {
        EXPECT_EQ(hashMap.contains(pair.first), true);
    }
}
