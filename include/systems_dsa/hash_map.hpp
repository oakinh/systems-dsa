#pragma once
#include <systems_dsa/vector.hpp>

namespace systems_dsa {

template <typename K, typename V>
struct Bucket {
    using value_type = std::pair<K, V>;
    enum class State : uint8_t {
        OPEN,
        FILLED,
        TOMBSTONE,
    };
    alignas(value_type) std::byte storage[sizeof(value_type)];
};

template <
    typename K,
    typename V,
    class Hasher = std::hash<K>,
    class KeyEqual = std::equal_to<K>
    >
class hash_map {
    vector<Bucket<K, V>> hashMap;
    size_t tombstones {};
    size_t size {};
    constexpr static float maxLoadFactor { 0.7f };
    Hasher hasher;
    KeyEqual eq;

    // Default constructor
    hash_map() = default;

    // Constructor with size
    hash_map(size_t n)
        : size{ n }
        {}

    // Copy constructor
    hash_map(const hash_map& hashMap) {

    }

    // TODO: Can I make this noexcept?
    // Move constructor
    hash_map(hash_map&& hashMap) {

    }

    // Copy assignment operator
    hash_map& operator=(const hash_map& hashMap) {

    }

    // Move assignment operator
    hash_map& operator=(hash_map&& hashMap) {

    }


};
}