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
    using value_type = std::pair<const K, V>;
    struct Bucket {
        // Data
        State state = State::OPEN;
        alignas(value_type) std::byte storage[sizeof(value_type)]; // Uninitialized memory

        // Member functions
        value_type* ptr() noexcept {
            return std::launder(reinterpret_cast<value_type*>(storage));
        }
        const value_type* ptr() const noexcept {
            return std::launder(reinterpret_cast<const value_type*>(storage));
        }
        K& key() noexcept {
            assert(state == State::FILLED);
            return ptr()->first;
        }
        V& val() noexcept {
            assert(state == State::FILLED);
            return ptr()->second;
        }
        const K& key() const noexcept {
            assert(state == State::FILLED);
            return ptr()->first;
        }
        const V& val() const noexcept {
            assert(state == State::FILLED);
            return ptr()->second;
        }
    };
    // Data
    vector<Bucket> m_buckets {};
    size_t m_tombstones {};
    size_t m_filled { 0 }; // Filled count only
    constexpr static float maxLoadFactor { 0.7f };
    Hasher m_hasher;
    KeyEqual m_eq;
    constexpr static std::size_t sentinelIndex { std::numeric_limits<std::size_t>::max() };

    // Member functions
    float getLoadFactor() const {
        return m_tombstones + m_filled / m_buckets.size();
    }

    std::size_t getKeyIndex(const K& key) const {
        std::size_t hashedKey { m_hasher(key) };
        return hashedKey > 0
            ? hashedKey % m_buckets.size()
            : 0;
    }
    // TODO: Write a template implementation function, that enables both const member func and non-const calling
    std::size_t probe(std::size_t index, const std::optional<K> key = std::nullopt) const {
        // Whether for inserting or seeking, probing stops on an OPEN bucket

        const std::size_t bucketSize { m_buckets.size() };
        assert(bucketSize > 0 && "bucketSize not greater than 0 in probe");

        bool failure = false;
        //while (iterations < bucketSize && m_buckets[index].state != State::OPEN) { // TODO: this could probably be a for loop
        for (std::size_t iterations {};
            iterations < bucketSize && m_buckets[index].state != State::OPEN;
            ++iterations, index = (index + 1) % bucketSize) {
            assert(index < bucketSize && "Index in probe not less than bucketSize");
            if (key.has_value() && m_buckets[index].state == State::OPEN) {
                // Did not find key
                failure = true;
                break;
            }
            if (
                key.has_value()
                && m_buckets[index].state == State::FILLED
                && m_buckets[index].key() == key.value()
                ) {
                // Found key
                break;
            }
        } // When key.has_value(), returns a sentinel value if we find an OPEN bucket
        // if (!key.has_value()) {
        //     assert(false && "Unreachable code reached in probe.");
        // }
        return failure ? sentinelIndex : index;
    }

    std::size_t probeForKey(const K& key) const {
        return probe(getKeyIndex(key), key);
    }

    std::size_t probeForInsert(const K& key) const {
        return probe(getKeyIndex(key));
    }

    template <typename vt>
    void insert_impl(vt&& pair) {
        std::size_t index { probeForInsert(pair.first) };

        if (m_buckets[index].state == State::OPEN) {
            new (m_buckets[index].storage) value_type(std::forward<value_type>(pair));

            std::cout << "Index given to placementNew in insert: " << index << '\n';
            std::cout << "bucketSize in placementNew in insert: " << m_buckets.size() << '\n';
            std::cout << "bucket capacity in placementNew in insert: " << m_buckets.capacity() << '\n';
            std::cout << "size() gives: " << size() << '\n';
            m_buckets[index].state = State::FILLED;
            ++m_filled;
        } else {
            assert(false && "State was not open during insert, this statement should not have been reached in insert");
        }
    }

public:
    // Default constructor
    hash_map() {
        m_buckets.resize(10);
    };

    // Constructor with size
    explicit hash_map(size_t n) {
        m_buckets.resize(n);
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
    void insert(const K& first, const V& second) {
        insert_impl(value_type{std::move(first), std::move(second)});
    }

    // TODO: R value K and V?

    void insert(const value_type& pair) {
        insert_impl(pair);
    }

    void insert(value_type&& pair) {
        insert_impl(std::move(pair));
    }

    V& find(const K& key) {
        std::size_t index { probeForKey(key) };
        return m_buckets[index].val();
    }
    const V& find(const K& key) const {
        std::size_t index { probeForKey(key) };
        return m_buckets[index].val();
    }

    bool contains(const K& key) const {
        return probeForKey(key) < m_buckets.size();
    }

    std::size_t size() const {
        return m_filled;
    }

    V& operator[](const K& key) {
        return find(key);
    }
    const V& operator[](const K& key) const {
        return find(key);
    }

    V& at(const K& key) {
        std::size_t index { find(key) };
        if (index >= size()) {
            throw std::out_of_range("The key provided was not found in the hashmap\n");
        }
        return m_buckets[index].val();
    }

    const V& at(const K& key) const {
        std::size_t index { find(key) };
        if (index >= size()) {
            throw std::out_of_range("The key provided was not found in the hashmap\n");
        }
        return m_buckets[index].val();
    }

};
}