#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>
#include "utils/lifetime_tracker.hpp"
#include "utils/throws_on_copy.hpp"

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

TEST(VectorTest, PopBackOnEmptyNoOps) {
    systems_dsa::vector<std::string> myVec {};
    size_t oldCapacity { myVec.capacity() };
    size_t oldSize { myVec.size() };
    myVec.pop_back();
    EXPECT_EQ(oldCapacity, myVec.capacity());
    EXPECT_EQ(oldSize, myVec.size());
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

///////////////////////
// Adversarial Tests //
///////////////////////
