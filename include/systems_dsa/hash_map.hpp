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
        alignas(value_type) std::byte storage[sizeof(value_type)]; // Uninitialized memory

        value_type* ptr() noexcept {
            return std::launder(reinterpret_cast<value_type*>(storage));
        }
        const value_type* ptr() const noexcept {
            return std::launder(reinterpret_cast<const value_type*>(storage));
        }

        K& key() noexcept { return ptr()->first; }
        V& val() noexcept { return ptr()->second; }
    };
    vector<Bucket> m_buckets;
    size_t m_tombstones {};
    size_t m_filled { 0 }; // Filled count only
    constexpr static float maxLoadFactor { 0.7f };
    Hasher m_hasher;
    KeyEqual m_eq;
    static constexpr std::size_t sentinelIndex { std::numeric_limits<std::size_t>::max() };
private:
    float getLoadFactor() const {
        return m_tombstones + m_filled / m_buckets.size();
    }

    std::size_t probe(std::optional<K> key = std::nullopt) {
        // Whether for inserting or seeking, probing stops on an OPEN bucket
        size_t iterations {};
        std::size_t bucketSize { m_buckets.size() };
        std::size_t index { key.has_value() ? m_hasher(key) % bucketSize : 0 };

        bool success = true;
        while (iterations < bucketSize && m_buckets[index].state != State::OPEN) {
            if (key.has_value() && m_buckets[index].state == State::OPEN) {
                // Did not find key
                success = false;
                break;
            }
            if (
                key.has_value()
                && m_buckets[index].state == State::FILLED
                && m_buckets[index].key() == key
                ) {
                // Found key
                break;
            }
            ++index;
            ++iterations;
            assert(iterations < bucketSize && "There was no OPEN bucket during probe\n");
            if (index >= bucketSize) {
                index = 0; // More performant than doing division every loop iteration
            }
        } // When key.has_value(), returns a sentinel value if we find an OPEN bucket
        return success ? index : sentinelIndex;
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

    // Move assignment operator
    hash_map& operator=(hash_map&& other) noexcept = default;

    ~hash_map() = default;

    ///////////////
    // Modifiers //
    ///////////////
    void insert(std::pair<K, V> pair) { // TODO: Figure out the overloads for this. r-value reference, forwarding reference, etc.
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
                //index = index % bucketSize;
                index = 0; // More performant than doing division every loop iteration
            }
        }

        if (m_buckets[index].state == State::OPEN) {
            new (m_buckets[index].storage) value_type(pair);
            m_buckets[index].state = State::FILLED;
        } else {
            std::cout << "State was not open during insert, this statement should not have been reached\n";
        }
    }
};
}