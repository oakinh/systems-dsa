#include <gtest/gtest.h>
#include <systems_dsa/vector.hpp>

TEST(VectorTest, BasicAssertions) {
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    EXPECT_EQ(myVec.size(), 2);
    EXPECT_GE(myVec.capacity(), 2);
    std::cout << "size: " << myVec.size() << " | capacity: " << myVec.capacity() << '\n';

    myVec.pop_back();

    EXPECT_EQ(myVec.size(), 1);
}