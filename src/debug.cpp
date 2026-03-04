#include <systems_dsa/vector.hpp>
#include <iostream>
#include <systems_dsa/hash_map.hpp>
#include <vector>

int main() {
	std::cout << "Hello programmers!\n";
    // systems_dsa::vector<int> myVec;
    // myVec.push_back(2);
    // myVec.push_back(8);
    //
    // std::cout << "size: " << myVec.size() << " | capacity: " << myVec.capacity() << '\n';
    // for (size_t i{}; i < myVec.size(); ++i) {
    //     std::cout << myVec[i] << '\n';
    //     // std::cout << i << '\n';
    // }
    //
    // myVec.pop_back();
    //
    // for (size_t i{}; i < myVec.size(); ++i) {
    //     std::cout << myVec[i] << '\n';
    // }
    //
    // systems_dsa::vector<int> anotherVec { 2 };
    //     myVec.resize(5);
    //     myVec[0] = 10;
    //     myVec[1] = 22;
    //     myVec.resize(10);

    systems_dsa::hash_map<int, int> hashMap { 4 };
    std::vector<std::pair<int, int>> pairsToInsert {
                    { 201, 101 },
                    { 203, 103 },
                    { 207, 107 },
                    { 210, 110 },
                    { 202, 102 },
                };
    for (const auto& p : pairsToInsert) {
        hashMap.insert(p);
    }

    std::cout << hashMap << '\n';

	return 0;
}
