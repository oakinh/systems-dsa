#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>
#include "utils/lifetime_tracker.hpp"
#include "utils/throws_on_copy.hpp"

TEST(VectorTest, PushBackIncreasesSizeCapacity) {
    [[maybe_unused]]ThrowsOnCopy myClass {};
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
    std::vector<std::string> strVec {
        "Hello",
        "World",
        "my name is bob the builder",
        "Boom",
        "Big explosion",
        "99999",
        "0000000",
        "111111111",
        "22222222222",
    };

    for (size_t i {}; i < strVec.size(); ++i) {
        myVec.push_back(strVec[i]);
    }

    EXPECT_GE(myVec.capacity(), strVec.size());
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
    systems_dsa::vector<LifetimeTracker> myVec {5 };
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
    systems_dsa::vector<LifetimeTracker> myVec {10 };
    LifetimeTracker::resetCounts();
    for (size_t i {}; i < 2; ++i) {
        myVec.emplace_back();
    }

    EXPECT_EQ(LifetimeTracker::moveCtorCount, 0);
    EXPECT_EQ(LifetimeTracker::ctorCount, 2);

    LifetimeTracker::resetCounts();
}

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