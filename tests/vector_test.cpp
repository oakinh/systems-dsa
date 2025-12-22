#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>
#include "utils/lifetime_tracker.hpp"

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
    int oldCapacity { myVec.capacity() };
    int oldSize { myVec.size() };
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

    for (int i {}; i < myVec.size(); ++i) {
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
    EXPECT_EQ(LifetimeTracker::ctorCount, 1);
    EXPECT_EQ(LifetimeTracker::copyAssignCount, 1);



    LifetimeTracker::resetCounts();
}