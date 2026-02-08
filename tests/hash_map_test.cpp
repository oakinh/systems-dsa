#include <gtest/gtest.h>
#include <systems_dsa/hash_map.hpp>

TEST(HashMapTest, DefaultConstructs) {
    using namespace std::string_literals;
    systems_dsa::hash_map<std::string, int> hashMap {};
    std::unordered_map<std::string, int> stdMap {};
    stdMap.insert({ "hello world", 5});
    hashMap.insert({"hello world"s, 5});

    stdMap.find()
}
