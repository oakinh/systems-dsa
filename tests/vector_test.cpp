#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>

TEST(VectorTest, PushBackIncreasesSizeCapacity) {
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    EXPECT_EQ(myVec.size(), 2);
    EXPECT_GE(myVec.capacity(), 2);
}

TEST(VectorTest, PopBackDecreasesSize) {
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    myVec.pop_back();
    EXPECT_EQ(myVec.size(), 1);
}