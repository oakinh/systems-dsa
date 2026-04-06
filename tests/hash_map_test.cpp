#include "utils/lifetime_tracker.hpp"
#include "utils/seed.hpp"
#include "utils/throws_on_copy.hpp"

#include <gtest/gtest.h>
#include <random>
#include <systems_dsa/hash_map.hpp>

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



///////////////////////////////
// Basic functionality tests //
///////////////////////////////

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

TEST(HashMapTest, ContainerUnmodifiedAfterReserveException) {
    systems_dsa::hash_map<std::size_t, ThrowsOnCopy> hashMap { 10 };
    ThrowsOnCopy::resetCounts();
    ThrowsOnCopy::throwOnInstance = 9;

    for (std::size_t i {}; i < 4; ++i) {
        EXPECT_NO_THROW(hashMap.insert(i, ThrowsOnCopy{})) << "insert threw for i=" << i;
    }

    // Throws on reserve
    EXPECT_ANY_THROW(hashMap.reserve(15));
    // Strong guarantee, size unchanged
    EXPECT_EQ(hashMap.size(), 4);

    for (std::size_t i {}; i < 4; ++i) {
        EXPECT_NO_THROW({
            auto* p = hashMap.find(i);
            EXPECT_NE(p, nullptr) << "find failed for key i=" << i;
        }) << "find threw for key i=" << i;
    }
    ThrowsOnCopy::resetCounts();
}

TEST(HashMapTest, ContainerUnmodifiedAfterInsertException) {
    systems_dsa::hash_map<std::size_t, ThrowsOnCopy> hashMap { 10 };
    ThrowsOnCopy::resetCounts();
    ThrowsOnCopy::throwOnInstance = 7;

    for (std::size_t i {}; i < 3; ++i) {
        EXPECT_NO_THROW(hashMap.insert(i, ThrowsOnCopy{}));
    }
    std::cout << "copyCtorCount" << ThrowsOnCopy::copyCtorCount << '\n';

    // Throws on insert
    EXPECT_ANY_THROW(hashMap.insert(3, ThrowsOnCopy{}));
    // Strong guarantee, size unchanged
    EXPECT_EQ(hashMap.size(), 3);
    for (std::size_t i {}; i < 3; ++i) {
        EXPECT_NO_THROW({
            auto* p = hashMap.find(i);
            EXPECT_NE(p, nullptr) << "find failed for key i=" << i;
        }) << "find threw for key i=" << i;
    }
    ThrowsOnCopy::resetCounts();

}

TEST_F(HashMapTest_F, DuplicateInsertNoOps) {
    std::vector<size_t> newValues { 1001, 1002, 1003, 1004, 1005 };
    for (std::size_t i{}; i < pairs.size(); ++i) {
        const auto& pair { pairs[i] };
        const auto& key { pair.first };
        hashMap.insert(key, newValues[i]);
        EXPECT_EQ(*hashMap.find(key), pair.second);
    }
}

/////////////////////////
// Adversarial testing //
/////////////////////////

TEST(HashMapTest, RandomSeqInsertEraseFindAgainstStd) {
    enum class OP: std::uint8_t {
        INSERT,
        ERASE,
        FIND,
    };

    std::uint64_t seed { getSeed("HASHMAP_SEED") };
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> distKeyVal(1, 1000);
    std::uniform_int_distribution<int> distOp(0, 2);

    systems_dsa::hash_map<int, int> hashMap {};
    std::unordered_map<int, int> reference {};

    for (std::size_t i {}; i < 10'000; ++i) {
        const int op { distOp(rng) };
        const int key { distKeyVal(rng) };
        const int val { distKeyVal(rng) };
        switch (static_cast<OP>(op)) {
        case OP::INSERT:
            hashMap.insert({ key, val });
            reference.insert({ key, val });
            break;
        case OP::ERASE:
            hashMap.erase(key);
            reference.erase(key);
            break;
        case OP::FIND:
            const auto* valPtr{ hashMap.find(key) };
            const auto& refIt { reference.find(key) };

            if (valPtr == nullptr) {
                if (refIt != reference.end()) {
                    FAIL();
                }
            } else {
                EXPECT_EQ(*valPtr, refIt->second);
            }
            EXPECT_EQ(hashMap.size(), reference.size());
            break;
        }
    }
}

TEST(HashMapTest, RandomSeqOperatorBracketsEraseFindAgainstStd) {
    enum class OP: std::uint8_t {
        OPERATOR_BRACKETS,
        ERASE,
        FIND,
    };
    std::uint64_t seed { getSeed("HASHMAP_SEED") };
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> distKeyVal(1, 40);
    std::uniform_int_distribution<int> distOp(0, 2);

    systems_dsa::hash_map<int, int> hashMap {};
    std::unordered_map<int, int> reference {};

    for (std::size_t i {}; i < 1000; ++i) {
        const int op { distOp(rng) };
        const int key { distKeyVal(rng) };
        const int val { distKeyVal(rng) };
        switch (static_cast<OP>(op)) {
        case OP::OPERATOR_BRACKETS:
            hashMap[key] = val;
            reference[key] = val;
            break;
        case OP::ERASE:
            hashMap.erase(key);
            reference.erase(key);
            break;
        case OP::FIND:
            const auto* valPtr{ hashMap.find(key) };
            const auto& refIt { reference.find(key) };

            if (valPtr == nullptr) {
                if (refIt != reference.end()) {
                    FAIL();
                }
            } else {
                EXPECT_EQ(*valPtr, refIt->second);
            }
            EXPECT_EQ(hashMap.size(), reference.size());
            break;
        }
    }
}

TEST(HashMapTest, RandomSeqEmplaceEraseFindAgainstStd) {
    enum class OP: std::uint8_t {
        EMPLACE,
        ERASE,
        FIND,
    };

    std::uint64_t seed { getSeed("HASHMAP_SEED") };
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> distKeyVal(1, 1000);
    std::uniform_int_distribution<int> distOp(0, 2);

    systems_dsa::hash_map<int, int> hashMap {};
    std::unordered_map<int, int> reference {};

    for (std::size_t i {}; i < 10'000; ++i) {
        const int op { distOp(rng) };
        const int key { distKeyVal(rng) };
        const int val { distKeyVal(rng) };
        switch (static_cast<OP>(op)) {
        case OP::EMPLACE:
            hashMap.emplace(key, val);
            reference.emplace(key, val);
            break;
        case OP::ERASE:
            hashMap.erase(key);
            reference.erase(key);
            break;
        case OP::FIND:
            const auto* valPtr{ hashMap.find(key) };
            const auto& refIt { reference.find(key) };

            if (valPtr == nullptr) {
                if (refIt != reference.end()) {
                    FAIL();
                }
            } else {
                EXPECT_EQ(*valPtr, refIt->second);
            }
            EXPECT_EQ(hashMap.size(), reference.size());
            break;
        }
    }
}

TEST(HashMapTest, HeavyTombstoneAccumulation) {
    systems_dsa::hash_map<int, int> hashMap {};
    std::size_t expectedFinalSize { 100 };
    for (int i {}; i < 10; ++i) {
        for (int k {}; k < static_cast<int>(hashMap.size()); ++k) {
            hashMap.erase(k);
        }
        for (int j {}; j < static_cast<int>(expectedFinalSize); ++j) {
            hashMap.insert(j, j + 100);
        }
    }
    EXPECT_EQ(hashMap.size(), expectedFinalSize) << "hashMap.size() != expectedFinalSize, and is instead: " << hashMap.size() << '\n';
}

TEST(HashMapTest, HeavyRepeatedClearing) {
    systems_dsa::hash_map<int, int> hashMap {};
    std::size_t expectedFinalSize { 100 };
    for (int i {}; i < 10; ++i) {
        hashMap.clear();
        EXPECT_EQ(hashMap.size(), 0);
        for (int j {}; j < static_cast<int>(expectedFinalSize); ++j) {
            EXPECT_FALSE(hashMap.contains(j) && "Clearing failed to destroy all elements");
            hashMap.insert(j, j + 100);
        }
    }
    EXPECT_EQ(hashMap.size(), expectedFinalSize) << "hashMap.size() != expectedFinalSize, and is instead: " << hashMap.size() << '\n';
}

TEST(HashMapTest, HeavyRehashing) {
    systems_dsa::hash_map<int, int> hashMap {};

    for (int i {}; i < 1000; ++i) {
        hashMap.insert({ i, i + 10 });
    }
    for (int i {}; i < 1000; ++i) {
        const auto* valPtr { hashMap.find(i) };
        EXPECT_NE(valPtr, nullptr);
        EXPECT_EQ(*valPtr, i + 10);
    }
}

TEST(HashMapTest, EverythingCollides) {
    struct ConstantHasher {
        std::size_t operator()(const int&) const noexcept {
            return 0;
        }
    };

    systems_dsa::hash_map<int, int, ConstantHasher> hashMap {};

    for (int i {}; i < 20; ++i) {
        hashMap.insert({ i, i + 10 });
    }
    hashMap.erase(0);
    hashMap.erase(2);
    hashMap.erase(3);
    hashMap.erase(4);

    EXPECT_EQ(*hashMap.find(1), 1 + 10);
    EXPECT_EQ(*hashMap.find(5), 5 + 10);
}
