#pragma once
#include <systems_dsa/vector.hpp>
#include <concepts>


namespace systems_dsa {

#ifndef NDEBUG
#define HM_ASSERT_VALID() assertValid()
#else
#define HM_ASSERT_VALID() ((void)0)
#endif

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
    vector<Bucket> m_buckets {};
    std::size_t m_tombstones {};
    std::size_t m_filled {}; // Filled count only
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
        return hashedKey > 0
            ? hashedKey % buckets.size()
            : 0;
    }

    std::size_t probeForKey(const K& key) const {
        std::size_t index { getKeyIndex(key) };
        const auto& buckets { m_buckets};
        const std::size_t bucketSize { buckets.size() };
        assert(bucketSize > 0 && "bucketSize not greater than 0 in probe");

        bool failure = false;
        for (std::size_t iterations {}; iterations < bucketSize; ++iterations, index = (index + 1) % bucketSize) {
            assert(index < bucketSize && "Index in probe not less than bucketSize");
            auto& bucket = buckets[index];
            State state = bucket.state;
            if (state == State::OPEN) {
                // We stop probing on OPEN buckets
                failure = true;
                break;
            } else if (state == State::FILLED && bucket.key() == key) {
                // Found key
                break;
            }
        }
        return failure ? sentinelIndex : index;
    }

    std::size_t probeForInsert(const K& key, const vector<Bucket>* bucketOverride = nullptr) const {
        std::size_t index { getKeyIndex(key, bucketOverride) };
        // Probing for insertion, probing stops on an OPEN or TOMBSTONE bucket

        const auto& buckets { bucketOverride ? *bucketOverride : m_buckets};
        const std::size_t bucketSize { buckets.size() };
        assert(bucketSize > 0 && "bucketSize not greater than 0 in probe");

        bool failure = false;
        std::size_t tombstoneIndex { sentinelIndex };
        for (std::size_t iterations {}; iterations < bucketSize; ++iterations, index = (index + 1) % bucketSize) {
            assert(index < bucketSize && "Index in probe not less than bucketSize");
            auto& bucket = buckets[index];
            State state = bucket.state;
            if (state == State::TOMBSTONE) {
                // We found a bucket suitable for insertion
                // We continue until an open bucket to ensure there's no duplicate keys
                tombstoneIndex = index;
            } else if (state == State::OPEN) {
                // We always stop probing on an OPEN bucket
                break;
            } else if (bucket.key() == key) {
                assert (state == State::FILLED);
                // Key already exists, no op
                failure = true;
            }
        }
        // assert((buckets[index].state == State::OPEN || buckets[index].state == State::TOMBSTONE)
        //         && "In probe forInsert == true, state was not OPEN or TOMBSTONE");
        if (tombstoneIndex != sentinelIndex) {
            // Provide the last found tombstone for insertion if found
            index = tombstoneIndex;
        }
        return failure ? sentinelIndex : index;
    }



    template <typename vt>
    Bucket* insert_impl(vt&& pair, vector<Bucket>* bucketOverride = nullptr) {
        // Rehash if necessary
        if (bucketOverride == nullptr && getLoadFactor(1) >= 0.70f) {
            rehash(m_buckets.size() * 2);

        }
        // Take the reference AFTER a potential rehash
        vector<Bucket>& buckets { bucketOverride ? *bucketOverride : m_buckets };
        std::size_t index { probeForInsert(pair.first, bucketOverride) };
        if (index == sentinelIndex) {
            // Duplicate key, no op
            return nullptr;
        }
        auto& bucket = buckets[index];
        State state = bucket.state;
        if (state != State::OPEN && state != State::TOMBSTONE) {
            std::cerr << "state is: " << static_cast<int>(state) << '\n';
            //assert(false && "State was not OPEN or TOMBSTONE during insert, this statement should not have been reached in insert");
            return nullptr;
        }
        // Placement-new

        new (bucket.storage) value_type(std::forward<vt>(pair));

        if (state == State::TOMBSTONE) {
            --m_tombstones;
        }
        bucket.state = State::FILLED;
        if (bucketOverride == nullptr) {
            ++m_filled;
        }

        HM_ASSERT_VALID();
        return &bucket;
    }

    std::size_t eraseAtIndex(std::size_t index, const std::optional<bool> clear = std::nullopt) noexcept {
        bool erased = false;
        if (index < m_buckets.size()) {
            auto& bucket { m_buckets[index] };
            if (bucket.state == State::FILLED) {
                bucket.ptr()->~value_type();
                if (clear.value_or(false)) {
                    // If we're clearing all elements, we set the state to OPEN
                    bucket.state = State::OPEN;
                } else {
                    bucket.state = State::TOMBSTONE;
                    ++m_tombstones;
                }

                --m_filled;
                erased = true;
            }
        }
        HM_ASSERT_VALID();
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
        HM_ASSERT_VALID();
    }

    // Constructor with size
    explicit hash_map(std::size_t n) {
        m_buckets.resize(n);
        for (std::size_t i {}; i < n; ++i) {
            assert(m_buckets[i].state == State::OPEN && "Default initialized bucket(s) were not OPEN");
        }
        HM_ASSERT_VALID();
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

    void insert(K&& first, V&& second) {
        insert_impl(value_type{ std::move(first), std::move(second) });
    }

    void insert(const K& first, const V& second) {
        insert_impl(value_type{ first, second });
    }

    void insert(const value_type& pair) {
        insert_impl(pair);
    }

    void insert(value_type&& pair) {
        insert_impl(std::move(pair));
    }

    // TODO: Implement piecewise-style emplacement
    // template <typename... Args>
    // requires std::constructible_from<value_type, Args&&...>
    // void emplace(Args&&... args) {
    //     insert_impl(std::forward<Args>(args)...);
    // }

    template <typename KArg, typename VArg>
    requires std::constructible_from<K, KArg&&> &&
        std::constructible_from<V, VArg&&>
    void emplace(KArg&& key, VArg&& value) {
        insert_impl(value_type(
            std::forward<KArg>(key),
            std::forward<VArg>(value)
            ));
    }

    V* find(const K& key) {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            return nullptr;
        }
        return index < m_buckets.size() ? &m_buckets[index].val() : nullptr;
    }
    const V* find(const K& key) const {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            return nullptr;
        }
        return index < m_buckets.size() ? &m_buckets[index].val() : nullptr;
    }

    bool contains(const K& key) const {
        return probeForKey(key) < m_buckets.size();
    }

    std::size_t size() const noexcept {
        return m_filled;
    }

    V& operator[](const K& key) {
        V* valPtr { find(key) };
        if (valPtr == nullptr) {
            Bucket* bucket { insert_impl(value_type(key, {})) };
            return bucket->val();
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

    bool empty() const noexcept {
        return m_filled == 0;
    }

    std::size_t erase(const K& key) {
        return eraseAtIndex(probeForKey(key));
    }

    // TODO: Add iterator support
    std::size_t erase(std::size_t index) noexcept {
        return eraseAtIndex(index);
    }

    // Exception guarantee:
    // Strong if type is copyable or nothrow movable
    // Otherwise basic only
    void rehash(std::size_t count) {
        vector<Bucket> newBuckets {};
        newBuckets.resize(count);
        try {
            for (std::size_t i{}; i < m_buckets.size(); ++i) {
                auto& oldBucket { m_buckets[i] };
                if (oldBucket.state == State::FILLED) {
                    insert_impl(std::move_if_noexcept(*oldBucket.ptr()), &newBuckets);
                }
            }
        } catch (...) {
            //for (const Bucket& newBucket : newBuckets) {
            for (std::size_t i {}; i < newBuckets.size(); ++i) {
                newBuckets[i].ptr()->~value_type();
            }
            throw;
        }
        m_tombstones = 0;
        m_buckets = std::move(newBuckets);
        HM_ASSERT_VALID();
    }

    void reserve(std::size_t count) {
        float loadFactorMultiplier { 1.3f }; // This ensures that rehashing isn't necessary to hold `count` elements
        rehash(std::ceil(count * loadFactorMultiplier) + 1);
    }

    void clear() {
        for (std::size_t i {}; i < m_buckets.size(); ++i) {
            eraseAtIndex(i, true);
        }
        HM_ASSERT_VALID();
    }


#ifndef NDEBUG
private:
    // This function checks numerous invariants of our hash_map, to assert that it is in a valid state.
    // We do this regardless of runtime overhead, in debug builds only.
    void assertValid() const {
        std::size_t tombstones {};
        std::size_t filled {};
        std::size_t open {};
        for (std::size_t i {}; i < m_buckets.size(); ++i) {
            const auto& bucket { m_buckets[i] };
            switch (bucket.state) {
            case State::OPEN:
                ++open;
                break;
            case State::FILLED:
                ++filled;
                break;
            case State::TOMBSTONE:
                ++tombstones;
                break;
            default:
                assert(false && "Unreachable code reached in assert valid switch statement - bucket.state default case");
            }
            if (bucket.state == State::FILLED) {
                const auto valFound { find(bucket.key()) };
                assert(valFound && "nullptr returned when attempting to find valid key");
                if (!(valFound == &bucket.val())) {
                    assert(false && "valFound did not equal expected value");
                }
            }
        }
        assert(tombstones == m_tombstones && "Tombstone count has drifted");
        assert(filled == m_filled && "Filled count has drifted");
        assert(open == m_buckets.size() - m_filled - m_tombstones && "Open count has drifted");
        assert(open > 0 && "Open count was not greater than zero");
        assert(getLoadFactor() == (filled + tombstones) / m_buckets.size() && "Load factor calculation is incorrect");
        assert(getLoadFactor() < maxLoadFactor && "Load factor has exceeded allowed maximum");
    }

public:
    template <class K2, class V2, class H2, class E2>
    friend std::ostream& operator<< (std::ostream&, const hash_map<K2, V2, H2, E2>&);
#endif
};

#ifndef NDEBUG
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
#endif

}