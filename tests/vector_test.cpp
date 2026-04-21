#include "utils/lifetime_tracker.hpp"
#include "utils/seed.hpp"
#include "utils/throws_on_copy.hpp"

#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>

///////////////////////////////
// Basic functionality tests //
///////////////////////////////

TEST(VectorTest, PushBackIncreasesSizeCapacity) {
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    EXPECT_EQ(myVec.size(), 2);
    EXPECT_GE(myVec.capacity(), 2);
}

TEST(VectorTest, EmptyIndicatesCorrectly) {
    systems_dsa::vector<int> myVec {};
    EXPECT_TRUE(myVec.empty());
    myVec.push_back(111);
    EXPECT_FALSE(myVec.empty());
}

TEST(VectorTest, PopBackDecreasesSize) {
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    myVec.pop_back();
    EXPECT_EQ(myVec.size(), 1);
}

TEST(VectorTest, ReserveEqFuncWithStdVec) {
    systems_dsa::vector<int> myVec {};
    myVec.reserve(10);
    std::vector<int> stdVec {};
    stdVec.reserve(10);

    EXPECT_EQ(myVec.capacity(), stdVec.capacity());
}

TEST(VectorTest, FailWhenDesiredCapacityIsLessThanCurrentCapacity) {
    systems_dsa::vector<int> myVec {};
    myVec.reserve(10);
    myVec.reserve(5);
    EXPECT_EQ(myVec.capacity(), 10);
}

TEST(VectorTest, ElementsIntactPostReserve) {
    systems_dsa::vector<std::string> myVec {};
    std::vector<std::string> strVec {
        "Hello",
        "World",
        "my name is bob the builder",
    };

    for (const auto& str : strVec) {
        myVec.push_back(str);
    }
    myVec.reserve(10);

    for (size_t i {}; i < myVec.size(); ++i) {
        EXPECT_EQ(myVec[i],strVec[i]);
    }
}

TEST(VectorTest, ContainerExpandsAfterPushes) {
    systems_dsa::vector<std::string> myVec {};
    size_t oldCapacity { myVec.capacity() };
    std::vector<std::string> strVec {
        "Hello",
        "World",
        "my name is bob",
        "Boom",
        "Big explosion",
        "Super",
        "cali",
        "fragilistic",
        "expialidocious",
    };

    for (size_t i {}; i < strVec.size(); ++i) {
        myVec.push_back(strVec[i]);
    }

    EXPECT_GE(myVec.capacity(), strVec.size());
    EXPECT_GE(myVec.capacity(), oldCapacity);
    for (size_t i {}; i < myVec.size(); ++i) {
        EXPECT_EQ(myVec[i], strVec[i]);
    }
}

TEST(VectorTest, ConstructionWorks) {
    systems_dsa::vector<LifetimeTracker> myVec {};
    LifetimeTracker::resetCounts();
    myVec.push_back(LifetimeTracker{});

    EXPECT_EQ(LifetimeTracker::moveCtorCount, 1);
    LifetimeTracker myTracker {};
    myVec.push_back(myTracker);
    EXPECT_EQ(LifetimeTracker::ctorCount, 2);
    EXPECT_EQ(LifetimeTracker::copyCtorCount, 1);

    LifetimeTracker::resetCounts();
}

TEST(VectorTest, MoveAndDestructionWorks) {
    systems_dsa::vector<LifetimeTracker> myVec(5);
    LifetimeTracker::resetCounts();
    for (int i {}; i < 3; ++i) {
        myVec.push_back(LifetimeTracker{});
    }
    EXPECT_EQ(LifetimeTracker::dtorCount, 3);
    EXPECT_EQ(LifetimeTracker::moveCtorCount, 3);

    myVec.reserve(10);

    EXPECT_EQ(LifetimeTracker::dtorCount, 6);
    EXPECT_EQ(LifetimeTracker::moveCtorCount, 6);

    LifetimeTracker::resetCounts();
}

TEST(VectorTest, EmplaceBackConstructsInContainer) {
    systems_dsa::vector<LifetimeTracker> myVec(10);
    LifetimeTracker::resetCounts();
    for (size_t i {}; i < 2; ++i) {
        auto& ref { myVec.emplace_back(i) };
        EXPECT_EQ(ref.id, i);
        EXPECT_TRUE(ref.isAlive());
    }

    EXPECT_EQ(LifetimeTracker::moveCtorCount, 0);
    EXPECT_EQ(LifetimeTracker::copyCtorCount, 0);
    EXPECT_EQ(LifetimeTracker::ctorCount, 2);

    LifetimeTracker::resetCounts();
}

TEST(VectorTest, ResizeDefaultConstructs) {
    systems_dsa::vector<int> myVec(2);
    std::size_t newSize1 { 5 };
    myVec.resize(newSize1);
    EXPECT_EQ(myVec.size(), newSize1);
    myVec[0] = 10;
    myVec[1] = 22;
    std::size_t newSize2 { 10 };
    myVec.resize(newSize2);
    EXPECT_EQ(myVec[0], 10);
    EXPECT_EQ(myVec[1], 22);
    for (size_t i { 2 }; i < myVec.size(); ++i) {
        EXPECT_EQ(myVec[i], 0);
    }
    EXPECT_EQ(myVec.size(), newSize2);
}

TEST(VectorTest, ResizeCorrectlyDecreasesSize) {
    systems_dsa::vector<int> myVec { 2, 76, 23, 45, 90 };
    myVec.resize(2);
    EXPECT_EQ(myVec.size(), 2);
    myVec.push_back(100);
    EXPECT_EQ(myVec[0], 2);
    EXPECT_EQ(myVec[1], 76);
    EXPECT_EQ(myVec[2], 100);
}

TEST(VectorTest, FrontReturnsFirstElement) {
    systems_dsa::vector<int> myVec { 1, 2, 3 };
    ASSERT_EQ(myVec.front(), myVec[0]);
}

TEST(VectorTest, BackReturnsLastElement) {
    systems_dsa::vector<int> myVec { 1, 2, 3 };
    ASSERT_EQ(myVec.back(), myVec[myVec.size() - 1]);
}

TEST(VectorTest, BackReturnsOnlyElement) {
    systems_dsa::vector<int> myVec { 1 };
    ASSERT_EQ(myVec.front(), myVec[myVec.size() - 1]);
}

TEST(VectorTest, SubscriptOperatorSupportsAssignment) {
    systems_dsa::vector<LifetimeTracker> myVec {};

    for (int i {}; i < 5; ++i) {
        myVec.push_back({ i });
    }
    EXPECT_EQ(myVec.size(), 5);
    EXPECT_GE(myVec.capacity(), 5);

    auto oldSize { myVec.size() };
    auto oldCapacity { myVec.capacity() };

    // Copy assignment
    LifetimeTracker::resetCounts();
    LifetimeTracker tracker { 12 };
    myVec[3] = tracker;
    EXPECT_EQ(LifetimeTracker::copyAssignCount, 1);
    EXPECT_EQ(myVec[3].id, tracker.id);

    // Move assignment
    LifetimeTracker::resetCounts();
    myVec[4] = LifetimeTracker { 15 };
    EXPECT_EQ(LifetimeTracker::moveAssignCount, 1);
    EXPECT_EQ(myVec[4].id, 15);

    EXPECT_EQ(oldSize, myVec.size());
    EXPECT_EQ(oldCapacity, myVec.capacity());
}

TEST(VectorTest, RangeBasedForLoopWorks) {
    systems_dsa::vector<int> myVec { 10, 20, 30, 40, 50, 60 };

    std::size_t i {};
    for (const auto& num : myVec) {
        ASSERT_EQ(num, myVec[i]);
        ++i;
    }
}

//////////////////////
// Exception Safety //
//////////////////////

TEST(VectorTest, ContainerUnmodifiedAfterException) {
    ThrowsOnCopy::resetCounts();
    systems_dsa::vector<ThrowsOnCopy> myVec;
    myVec.reserve(8);
    ThrowsOnCopy::throwOnInstance = 6;
    if (myVec.capacity() != 8) {
        FAIL() << "Capacity did not equal 8";
    }
    for (int i {}; i < 4; ++i) {
        ThrowsOnCopy myObj { i };
        myVec.push_back(myObj);
    }
    ASSERT_EQ(ThrowsOnCopy::copyCtorCount, 4);
    EXPECT_ANY_THROW(myVec.reserve(10));

    try {
        for (size_t i {}; i < myVec.size(); ++i) {
            EXPECT_EQ(myVec[i].id, static_cast<int>(i));
        }
    } catch (const std::exception& e) {
        FAIL() << "Escaped exception: " << e.what();
    }
}

///////////////////////
// Lifetime Tracking //
///////////////////////

TEST(VectorTest, PopBackDestroysElement) {
    systems_dsa::vector<LifetimeTracker> myVec {};

    myVec.push_back({});
    ASSERT_TRUE(myVec[0].isAlive());
    LifetimeTracker::resetCounts();
    myVec.pop_back();
    EXPECT_EQ(LifetimeTracker::dtorCount, 1);
    LifetimeTracker::resetCounts();
}

TEST(VectorTest, DestructorDestroysElements) {
    {
        systems_dsa::vector<LifetimeTracker> myVec {};
        for (std::size_t i {}; i < 10; ++i) {
            myVec.push_back({});
            ASSERT_TRUE(myVec[i].isAlive());
        }
        LifetimeTracker::resetCounts();
    }
    EXPECT_EQ(LifetimeTracker::dtorCount, 10);
}

TEST(VectorTest, SmallerResizeCallsDestructors) {
    systems_dsa::vector<LifetimeTracker> myVec { 10, 20 ,30 , 40, 50, 60 ,70, 80 };
    EXPECT_EQ(myVec.size(), 8);
    LifetimeTracker::resetCounts();
    myVec.resize(4);
    EXPECT_EQ(myVec.size(), 4);
    EXPECT_EQ(LifetimeTracker::dtorCount, 4);
}

///////////////////////
// Adversarial Tests //
///////////////////////

TEST(VectorTest, RandomSeqEmplacePopResizeAccessAgainstStd) {
    enum class OP : std::uint8_t {
        EMPLACE,
        POP,
        RESIZE,
        ACCESS,
    };

    std::uint64_t seed { getSeed("VECTOR_SEED") };
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> distVal(1, 1000);
    std::uniform_int_distribution<int> distOp(0, 3);

    systems_dsa::vector<LifetimeTracker> vec {};
    std::vector<LifetimeTracker> reference {};

    for (std::size_t i {}; i < 10'000; ++i) {
        EXPECT_EQ(vec.empty(), reference.empty());

        // Force insertion if empty
        const int op { vec.empty() ? static_cast<int>(OP::EMPLACE) : distOp(rng) };
        const int val { distVal(rng) };

        switch (static_cast<OP>(op)) {
        case OP::EMPLACE: {
                const auto& vecVal { vec.emplace_back(val) };
                const auto& refVal { reference.emplace_back(val) };
                EXPECT_EQ(vecVal.id, refVal.id);
                EXPECT_TRUE(vecVal.isAlive() && refVal.isAlive());
                EXPECT_EQ(vec.size(), reference.size());
                break;
            }
        case OP::POP:
            if (vec.empty()) FAIL() << "Unreachable test code reached";
            vec.pop_back();
            reference.pop_back();
            EXPECT_EQ(vec.size(), reference.size());
            break;
        case OP::RESIZE:
            vec.resize(val);
            reference.resize(val);
            EXPECT_EQ(vec.size(), reference.size());
            break;
        case OP::ACCESS:
            if (!vec.empty()) {
                EXPECT_EQ(vec.front().id, reference.front().id);
                EXPECT_EQ(vec.back().id, reference.back().id);
            }
            for (std::size_t j {}; j < vec.size(); ++j) {
                EXPECT_EQ(vec[j].id, reference[j].id);
                EXPECT_TRUE(vec[j].isAlive() && reference[j].isAlive());
            }
            break;
        }
    }
}