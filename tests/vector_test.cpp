#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>

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