#include <systems_dsa/vector.hpp>
#include <iostream>

int main() {
	std::cout << "Hello programmers!\n";
    systems_dsa::vector<int> myVec;
    myVec.push_back(2);
    myVec.push_back(8);

    std::cout << "size: " << myVec.size() << " | capacity: " << myVec.capacity() << '\n';
    for (int i{}; i < myVec.size(); ++i) {
        std::cout << myVec[i] << '\n';
        // std::cout << i << '\n';
    }

    myVec.pop_back();

    for (int i{}; i < myVec.size(); ++i) {
        std::cout << myVec[i] << '\n';
    }

	return 0;
}
