#include <gtest/gtest.h>
#include <src/array/vector.hpp>

TEST(VectorTest, BasicAssertions) {
    oakin::Vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    std::cout << "size: " << myVec.size() << " | capacity: " << myVec.capacity() << '\n';
    for (size_t i{}; i < myVec.size(); ++i) {
        std::cout << myVec[i] << '\n';
        // std::cout << i << '\n';
    }

    myVec.pop_back();

    for (size_t i{}; i < myVec.size(); ++i) {
        std::cout << myVec[i] << '\n';
    }
}