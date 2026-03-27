#pragma once
#include <systems_dsa/vector.hpp>
#include <concepts>

namespace systems_dsa {

template <typename H, typename K>
concept ValidHasher =
    std::regular_invocable<H, const K&> &&
        std::convertible_to<std::invoke_result_t<H, const K&>, std::size_t>;

template <typename Eq, typename K>
concept ValidKeyEqual =
    std::predicate<Eq, const K&, const K&>;

// Forward declaration
template <
    typename K,
    typename V,
    class Hasher = std::hash<K>,
    class KeyEqual = std::equal_to<K>
    >
requires ValidHasher<Hasher, K> &&
    ValidKeyEqual<KeyEqual, K>
class hash_map;

// Forward declaration
template <class K, class V, class Hash, class KeyEq>
std::ostream& operator<<(std::ostream& out,
                         const hash_map<K, V, Hash, KeyEq>& hashMap);

// Start of class
template <
    typename K,
    typename V,
    class Hasher,
    class KeyEqual
    >
requires ValidHasher<Hasher, K> &&
    ValidKeyEqual<KeyEqual, K>
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
    //std::vector<Bucket> m_buckets {};
    vector<Bucket> m_buckets {};
    size_t m_tombstones {};
    size_t m_filled {}; // Filled count only
    constexpr static float maxLoadFactor { 0.7f };
    Hasher m_hasher;
    KeyEqual m_eq;
    constexpr static std::size_t sentinelIndex { std::numeric_limits<std::size_t>::max() };

    // Member functions
    float getLoadFactor(const std::optional<std::size_t> additions = std::nullopt) const {
        return (m_tombstones + m_filled + additions.value_or(0)) / m_buckets.size();
    }

    std::size_t getKeyIndex(const K& key, const vector<Bucket>* bucketOverride = nullptr) const {
        const auto& buckets { bucketOverride ? *bucketOverride : m_buckets };
        std::size_t hashedKey { m_hasher(key) };
        std::cout << "hashedKey: " << hashedKey << " - buckets.size(): " << buckets.size() << '\n';
        return hashedKey > 0
            ? hashedKey % buckets.size()
            : 0;
    }
    // TODO: Write a template implementation function, that enables both const member func and non-const calling
    std::size_t probe(
        std::size_t index,
        const std::optional<K> key = std::nullopt,
        const vector<Bucket>* bucketOverride = nullptr
        ) const {
        // For finding a key, probing stops only on an OPEN bucket
        // Probing for insertion, probing stops on an OPEN or TOMBSTONE bucket
        std::cout << "Index in probe: " << index << " - key in probe: " << key.value_or(0) << '\n';
        const auto& buckets { bucketOverride ? *bucketOverride : m_buckets};
        bool forInsert = !key.has_value();
        const std::size_t bucketSize { buckets.size() };
        assert(bucketSize > 0 && "bucketSize not greater than 0 in probe");
        std::cout << "bucketSize: " << bucketSize << '\n';
        bool failure = false;
        for (std::size_t iterations {}; iterations < bucketSize; ++iterations, index = (index + 1) % bucketSize) {
            assert(index < bucketSize && "Index in probe not less than bucketSize");
            auto& bucket = buckets[index];
            State state = bucket.state;
            if (state == State::OPEN) {
                // We stop probing on OPEN buckets in both use cases
                // Either we didn't find the key we're looking for (failure == true)
                // Or we found an OPEN bucket suitable for insertion
                if (!forInsert) failure = true;
                break;
            }
            if (
                !forInsert
                && state == State::FILLED
                && bucket.key() == key.value()
                ) {
                // Found key
                break;
            }

            if (forInsert && state == State::TOMBSTONE) {
                // Found bucket for inserting
                break;
            }
            std::cout << "iterations: " << iterations << '\n';
        } // When key.has_value(), returns a sentinel value if we find an OPEN bucket
        // if (!key.has_value()) {
        //     assert(false && "Unreachable code reached in probe.");
        // }
        if (forInsert) {
            assert((buckets[index].state == State::OPEN || buckets[index].state == State::TOMBSTONE)
                && "In probe forInsert == true, state was not OPEN or TOMBSTONE");
            assert(!failure);
        }
        return failure ? sentinelIndex : index;
    }

    std::size_t probeForKey(const K& key) const {
        return probe(getKeyIndex(key), key);
    }

    std::size_t probeForInsert(const K& key, const vector<Bucket>* bucketOverride = nullptr) const {
        return probe(getKeyIndex(key, bucketOverride), std::nullopt, bucketOverride);
    }

    template <typename vt>
    Bucket* insert_impl(vt&& pair, vector<Bucket>* bucketOverride = nullptr) {
        if (bucketOverride == nullptr && getLoadFactor(1) >= 0.70f) {
            std::cout << "**REHASHING**\n";
            rehash(m_buckets.size() * 2);

        }
        // Take the reference AFTER a potential rehash
        vector<Bucket>& buckets { bucketOverride ? *bucketOverride : m_buckets };
        std::size_t index { probeForInsert(pair.first, bucketOverride) };
        auto& bucket = buckets[index];
        State state = bucket.state;
        if (state != State::OPEN && state != State::TOMBSTONE) {
            std::cerr << "state is: " << static_cast<int>(state) << '\n';
            assert(false && "State was not OPEN or TOMBSTONE during insert, this statement should not have been reached in insert");
            return nullptr;
        }
        // Placement-new
        new (bucket.storage) value_type(std::forward<vt>(pair));

        std::cout << "Index given to placementNew in insert: " << index << '\n';
        std::cout << "bucketSize in placementNew in insert: " << buckets.size() << '\n';
        std::cout << "bucket capacity in placementNew in insert: " << buckets.capacity() << '\n';
        std::cout << "size() gives: " << size() << '\n';
        if (state == State::TOMBSTONE) {
            --m_tombstones;
        }
        bucket.state = State::FILLED;
        if (bucketOverride == nullptr) {
            ++m_filled;
            std::cout << "Insert successful of: " << pair.first << '\n';
        } else {
            std::cout << "REinsert successful of: " << pair.first << '\n';
        }

        return &bucket;
    }

    std::size_t eraseAtIndex(std::size_t index) noexcept {
        bool erased = false;
        if (index < m_buckets.size()) {
            auto& bucket { m_buckets[index] };
            if (bucket.state == State::FILLED) {
                bucket.ptr()->~value_type();
                bucket.state = State::TOMBSTONE;
                ++m_tombstones;
                --m_filled;
                erased = true;
            }
        }
        return erased ? 1 : 0;
    }

public:
    // Default constructor
    hash_map() {
        m_buckets.resize(10);
        for (std::size_t i {}; i < 10; ++i) {
            assert(m_buckets[i].state == State::OPEN && "Default initialized bucket(s) were not OPEN");
        }
        assert(m_buckets.size() > 0 && "Default construction was not successful");
    }

    // Constructor with size
    explicit hash_map(std::size_t n) {
        m_buckets.resize(n);
        for (std::size_t i {}; i < n; ++i) {
            assert(m_buckets[i].state == State::OPEN && "Default initialized bucket(s) were not OPEN");
        }
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
    void insert(K first, V second) {
        insert_impl(value_type{std::move(first), std::move(second)});
    }
    // TODO: R value K and V?
    // TODO: Emplace()

    void insert(const value_type& pair) {
        insert_impl(pair);
    }

    void insert(value_type&& pair) {
        insert_impl(std::move(pair));
    }

    template <typename... Args>
    requires std::constructible_from<value_type, Args&&...>
    void emplace(Args&&... args) {
        insert_impl(std::forward<Args>(args)...);
    }

    V* find(const K& key) {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            std::cout << "index: " << index << " - nullptr placeholder\n";
            return nullptr;
        }
        return index < m_buckets.size() ? &m_buckets[index].val() : nullptr;
    }
    const V* find(const K& key) const {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            std::cout << "index: " << index << " - nullptr placeholder\n";
            return nullptr;
        }
        return index < m_buckets.size() ? &m_buckets[index].val() : nullptr;
    }

    bool contains(const K& key) const {
        return probeForKey(key) < m_buckets.size();
    }

    std::size_t size() const {
        return m_filled;
    }

    V& operator[](const K& key) {
        const V* valPtr { find(key) };
        if (valPtr == nullptr) {
            return *insert_impl({ {}, {} });
        }
        return *valPtr;
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

    bool empty() const {
        return m_filled == 0;
    }

    std::size_t erase(const K& key) {
        return eraseAtIndex(probeForKey(key));
    }

    // TODO: Add iterator support
    std::size_t erase(std::size_t index) {
        return eraseAtIndex(index);
    }

    void rehash(std::size_t count) {
        vector<Bucket> newBuckets {};
        newBuckets.resize(count);
        try {
            // for (const Bucket& oldBucket : m_buckets) {
            //     if (oldBucket.state == State::FILLED) {
            //         insert(std::move_if_noexcept(*oldBucket.ptr()), &newBuckets);
            //     }
            // }
            std::cout << "**START REHASH**\n";
            for (std::size_t i{}; i < m_buckets.size(); ++i) {
                auto& oldBucket { m_buckets[i] };
                if (oldBucket.state == State::FILLED) {
                    insert_impl(std::move_if_noexcept(*oldBucket.ptr()), &newBuckets);
                }
            }

        } catch (...) {
            //for (const Bucket& newBucket : newBuckets) {
            for (std::size_t i {}; i< newBuckets.size(); ++i) {
                newBuckets[i].ptr()->~value_type();
            }
            throw;
        }
        m_tombstones = 0;
        m_buckets = std::move(newBuckets);
        std::cout << "**END REHASH**\n";
    }

    void reserve(std::size_t count) {
        float loadFactorMultiplier { 1.3f }; // This ensures that rehashing isn't necessary to hold `count` elements
        rehash(std::ceil(count * loadFactorMultiplier) + 1);
    }

    void clear() noexcept {
        for (std::size_t i {}; i < m_buckets.size(); ++i) {
            eraseAtIndex(i);
        }
    }

    template <class K2, class V2, class H2, class E2>
    friend std::ostream& operator<< (std::ostream&, const hash_map<K2, V2, H2, E2>&);
};
template <class K, class V, class Hash, class KeyEq>
std::ostream& operator<< (std::ostream& out,
    const hash_map<K, V, Hash, KeyEq>& hashMap) {
    out << "[";
    for (std::size_t i {}; i < hashMap.m_buckets.size(); ++i) {
        if (i) out << ", ";
        out << i << ": ";
        const auto& bucket { hashMap.m_buckets[i] };
        if (bucket.state == hash_map<K, V, Hash, KeyEq>::State::FILLED) {
            out << "{ " << bucket.key() << ", " << bucket.val() << " }";
        } else {
            out << "empty";
        }
        out << '\n';
    }
    out << "]";
    return out;
}
}