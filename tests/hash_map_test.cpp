#include "utils/lifetime_tracker.hpp"
#include "utils/seed.hpp"
#include "utils/throws_on_copy.hpp"

#include <gtest/gtest.h>
#include <random>
#include <unordered_map>
#include <string>
#include <cctype>
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

class HashMapTest_LT_F : public testing::Test {
protected:
    systems_dsa::hash_map<int, LifetimeTracker> hashMap {};
    std::vector<std::pair<int, LifetimeTracker>> pairs {
                    { 1, {} },
                    { 3, {} },
                    { 7, {} },
                    { 10, {} },
                    { 2, {} },
                    { 12, {} },
                    { 13, {} },
                    { 17, {} },
                    { 110, {} },
                    { 19, {} },
                };
    HashMapTest_LT_F() {
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

TEST(HashMapTest, InsertReturnsValidIterator) {
    systems_dsa::hash_map<int, int> hashMap {};
    auto insertReturn { hashMap.insert({ 1, 10 }) };
    EXPECT_EQ(hashMap.find(insertReturn.first->first), insertReturn.first);
}

TEST_F(HashMapTest_F, FindReturnsCorrectValue) {
    for (const auto& pair : pairs) {
        const auto it { hashMap.find(pair.first) };
        if (it == hashMap.end()) FAIL() << "it was end(), first: " << pair.first;
        EXPECT_EQ(hashMap.find(pair.first)->second, pair.second);
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
    LifetimeTracker::resetCounts();
}

TEST_F(HashMapTest_F, RehashLosesNoElements) {
    for (const auto& p : pairs) {
        const auto it { hashMap.find(p.first) };
        if (it == hashMap.end()) FAIL() << "it was end(), first: " << p.first;
        EXPECT_EQ(hashMap.find(p.first)->second, p.second);
    }
}

TEST_F(HashMapTest_LT_F, ClearDestroysAllElements) {
    LifetimeTracker::resetCounts();
    EXPECT_EQ(LifetimeTracker::dtorCount, 0);
    hashMap.erase(pairs[2].first);
    hashMap.clear();
    EXPECT_EQ(LifetimeTracker::dtorCount, pairs.size());
    EXPECT_EQ(hashMap.size(), 0);
    for (const auto& p : pairs) {
        EXPECT_FALSE(hashMap.contains(p.first));
    }
    LifetimeTracker::resetCounts();
}

TEST(HashMapTest, ExceptionDuringRehashPreservesContainer) {
    ThrowsOnCopy::resetCounts();
    ThrowsOnCopy::throwOnInstance = 15;
    systems_dsa::hash_map<int, ThrowsOnCopy> hashMap {};
    hashMap.reserve(6);
    for (int i {}; i < 6; ++i) {
        hashMap.insert({ i, {} });
    }

    ASSERT_EQ(ThrowsOnCopy::copyCtorCount, 12);
    ASSERT_EQ(ThrowsOnCopy::dtorCount, 6);
    EXPECT_ANY_THROW(hashMap.rehash(12));
    EXPECT_EQ(ThrowsOnCopy::dtorCount, 9);

    ThrowsOnCopy::resetCounts();
}

TEST_F(HashMapTest_F, ElementsIntactPostReserve) {
    std::size_t hashMapSize { hashMap.size() };
    hashMap.reserve(30);
    EXPECT_EQ(hashMap.size(), hashMapSize);
    for (const auto& pair : pairs) {
        EXPECT_EQ(pair.second, hashMap.find(pair.first)->second);
    }
}

TEST(HashMapTest, ReservedMapAllowsCountElementsWithoutRehash) {
    systems_dsa::hash_map<int, int> hashMap {};
    hashMap.reserve(100);
    std::size_t bucketCount { hashMap.bucket_count() };
    for (std::size_t i {}; i < 100; ++i) {
        hashMap.insert({ i, {} });
    }
    EXPECT_EQ(bucketCount, hashMap.bucket_count()) << "Reserve didn't guarantee the map could hold N elements without a rehash";

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

    for (std::size_t i {}; i < hashMap.size(); ++i) {
        EXPECT_NO_THROW({
            auto it = hashMap.find(i);
            EXPECT_NE(it, hashMap.end()) << "find failed for key i=" << i;
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
            auto it = hashMap.find(i);
            EXPECT_NE(it, hashMap.end()) << "find failed for key i=" << i;
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
        EXPECT_EQ(hashMap.find(key)->second, pair.second);
    }
}

TEST_F(HashMapTest_F, IteratorPreIncrementTraversal) {
    std::size_t i {};
    for (auto it = hashMap.begin(); it != hashMap.end(); ++it, ++i) {
        EXPECT_NE(hashMap.find(it->first), hashMap.end());
        EXPECT_EQ(hashMap.find(it->first)->second, it->second);
        if (i == hashMap.size()) {
            FAIL() << "Iterator failed to reach end, terminating \n";
        }
    }
}

TEST_F(HashMapTest_F, IteratorPostIncrementTraversal) {
    std::size_t i {};
    for (auto it = hashMap.begin(); it != hashMap.end(); it++) {
        EXPECT_NE(hashMap.find(it->first), hashMap.end());
        EXPECT_EQ(hashMap.find(it->first)->second, it->second);
        if (i == hashMap.size()) {
            FAIL() << "Iterator failed to reach end, terminating \n";
        }
    }
}

TEST_F(HashMapTest_F, RangeBasedLoop) {
    for (const auto& pair : hashMap) {
        EXPECT_NE(hashMap.find(pair.first), hashMap.end());
        EXPECT_EQ(hashMap.find(pair.first)->second, pair.second);
    }
}

TEST (HashMapTest, CorrectlyChecksEqualityWithKeyEqual) {
    struct CaseInsensitiveEqual {
        bool operator()(const std::string& lhs, const std::string& rhs) const {
            if (lhs.length() != rhs.length()) return false;

            for (std::size_t i {}; i < lhs.length(); ++i) {
                if (std::tolower(lhs[i]) != std::tolower(rhs[i])) return false;
            }
            return true;
        }
    };

    struct CaseInsensitiveHash {
        std::size_t operator()(const std::string& str) const {
            std::size_t hash = 0;
            for (char c : str) {
                hash = hash * 31 + std::tolower(c);
            }
            return hash;
        }
    };

    systems_dsa::hash_map<std::string, int, CaseInsensitiveHash, CaseInsensitiveEqual> hashMap {};

    hashMap.insert("abc", 1);
    auto [ it, success] = hashMap.insert("ABC", 2);
    EXPECT_FALSE(success);
    EXPECT_EQ(it->second, 1);
    EXPECT_TRUE(hashMap.contains("ABC"));
    EXPECT_EQ(hashMap.erase("ABC"), 1);
}

TEST(HashMapTest, GrowthPolicyEnforced) {
    systems_dsa::hash_map<int, int> hashMap { 10 };

    EXPECT_EQ(hashMap.bucket_count(), 10);

    for (std::size_t i{}; i < 7; ++i) {
        hashMap.insert({ i, {} });
    }
    EXPECT_GT(hashMap.bucket_count(), 10);
}

TEST_F(HashMapTest_LT_F, RehashDestroysOldElements) {
    LifetimeTracker::resetCounts();
    hashMap.rehash(hashMap.bucket_count() + 20);
    EXPECT_EQ(LifetimeTracker::dtorCount, hashMap.size());
    LifetimeTracker::resetCounts();
}

TEST(HashMapTest, DestructorDestroysElements) {
    LifetimeTracker::resetCounts();
    {
        systems_dsa::hash_map<int, LifetimeTracker> hashMap;
        for (std::size_t i{}; i < 5; ++i) {
            hashMap.insert({ i, {} });
        }
        EXPECT_EQ(LifetimeTracker::liveCount, 5);
    }
    EXPECT_EQ(LifetimeTracker::liveCount, 0);
    LifetimeTracker::resetCounts();
}

TEST_F(HashMapTest_F, ReserveDoesntAllowShrinking) {
    std::size_t bucketCount { hashMap.bucket_count() };
    hashMap.reserve(1);
    EXPECT_EQ(bucketCount, hashMap.bucket_count());
}

TEST_F(HashMapTest_F, RehashDoesntAllowShrinking) {
    std::size_t bucketCount { hashMap.bucket_count() };
    hashMap.rehash(1);
    EXPECT_EQ(bucketCount, hashMap.bucket_count());
}

TEST(HashMapTest, ConstructorWithZeroThrows) {
    using hash_map = systems_dsa::hash_map<int, int>;
    EXPECT_ANY_THROW(hash_map hashMap{0 });
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
            const auto it{ hashMap.find(key) };
            const auto refIt { reference.find(key) };

            if ((it == hashMap.end()) != (refIt == reference.end())) {
                FAIL();
            } else if (it != hashMap.end()) {
                EXPECT_EQ(it->second, refIt->second);
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
            const auto it{ hashMap.find(key) };
            const auto refIt { reference.find(key) };

            if ((it == hashMap.end()) != (refIt == reference.end())) {
                FAIL();
            } else if (it != hashMap.end()) {
                EXPECT_EQ(it->second, refIt->second);
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
            const auto it{ hashMap.find(key) };
            const auto refIt { reference.find(key) };

            if ((it == hashMap.end()) != (refIt == reference.end())) {
                FAIL();
            } else if (it != hashMap.end()) {
                EXPECT_EQ(it->second, refIt->second);
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
        hashMap.erase(1); // Erase to ensure tombstones are also properly cleared
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
        const auto it { hashMap.find(i) };
        EXPECT_EQ(it->second, i + 10);
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

    EXPECT_EQ(hashMap.find(1)->second, 1 + 10);
    EXPECT_EQ(hashMap.find(5)->second, 5 + 10);
}

TEST(HashMapTest, IteratorTraversalWithTombstonesAndOpen) {
    struct IntHasher {
        std::size_t operator()(const int& key) const noexcept {
            return key;
        }
    };
    systems_dsa::hash_map<int, int, IntHasher> hashMap {};

    for (int i {}; i < 10; ++i) {
        if (i == 7) continue;
        hashMap.insert(i, i + 10);
    }
    hashMap.erase(3);
    hashMap.erase(4);
    hashMap.erase(8);

    int filledCount {};
    for (const auto& pair : hashMap) {
        EXPECT_TRUE(hashMap.contains(pair.first));
        ++filledCount;
    }
    EXPECT_EQ(filledCount, hashMap.size());
}
