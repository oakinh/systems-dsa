#pragma once
#include <systems_dsa/vector.hpp>

namespace systems_dsa {


template <
    typename K,
    typename V,
    class Hasher = std::hash<K>,
    class KeyEqual = std::equal_to<K>
    >
class hash_map {
    enum class State : uint8_t {
        OPEN,
        FILLED,
        TOMBSTONE,
    };
    using value_type = std::pair<K, V>;
    struct Bucket {
        State state = State::OPEN;
        alignas(value_type) std::byte storage[sizeof(value_type)];
    };
    vector<Bucket> m_buckets;
    size_t m_tombstones {};
    size_t m_filled { 0 }; // Filled count only
    constexpr static float maxLoadFactor { 0.7f };
    Hasher m_hasher;
    KeyEqual m_eq;
private:
    float getLoadFactor() const {
        return m_tombstones + m_filled / m_buckets.size();
    }
public:
    // Default constructor
    hash_map() = default;

    // Constructor with size
    explicit hash_map(size_t n) {
        m_buckets.resize(4);
    }

    // Copy constructor
    hash_map(const hash_map& other) = delete;

    // Move constructor
    hash_map(hash_map&& other) noexcept = default;

    // Copy assignment operator
    hash_map& operator=(const hash_map& other) = delete;

    // hash_map myhashmap {}
    // hash_map someotherhashMap {};
    // hash_map myhashmap = someotherhashmap
    // Move assignment operator
    hash_map& operator=(hash_map&& other) noexcept = default;

    ~hash_map() = default;

    ///////////////
    // Modifiers //
    ///////////////
    void insert(std::pair<K, V> pair) {
        std::size_t hashed { m_hasher(pair.first) };
        std::size_t bucketSize { m_buckets.size() };
        std::size_t index { hashed % bucketSize };

        Bucket bucket {
            State::FILLED,
            pair
        };
        size_t iterations {};
        while (iterations < bucketSize && m_buckets[index].state != State::OPEN) {
            ++index;
            ++iterations;
            assert(iterations < bucketSize && "There was no OPEN bucket to insert into\n");
            if (index >= bucketSize) {
                index = index % bucketSize;
            }
        }

        if (m_buckets[index].state == State::OPEN) {
            m_buckets[index] = std::move_if_noexcept(bucket);
        }
    }
};
}