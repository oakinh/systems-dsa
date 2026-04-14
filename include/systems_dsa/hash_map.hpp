#pragma once
#include <systems_dsa/vector.hpp>
#include <concepts>
#include <new>

// "DONE" Checklist
// TODO: spec cleanup
// TODO: One or two more adversarial tests

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

#ifndef NDEBUG
// Forward declaration
template <class K, class V, class Hash, class KeyEq>
std::ostream& operator<<(std::ostream& out,
                         const hash_map<K, V, Hash, KeyEq>& hashMap);
#endif

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
    template <bool isConst>
    class iterator_impl;
public:
    using iterator = iterator_impl<false>;
    using const_iterator = iterator_impl<true>;
private:
    //////////////////
    // Data Members //
    //////////////////
    vector<Bucket> m_buckets {};
    std::size_t m_tombstones {};
    std::size_t m_filled {};
    Hasher m_hasher;
    KeyEqual m_eq;
    constexpr static float maxLoadFactor { 0.7f };
    constexpr static std::size_t sentinelIndex { std::numeric_limits<std::size_t>::max() }; // TODO: Refactor to using m_buckets.size()

    // Member functions
    double getLoadFactor(std::size_t additions = 0) const {
        assert(!m_buckets.empty());
        return static_cast<double>(m_tombstones + m_filled + additions) / static_cast<double>(m_buckets.size());
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
        const auto& buckets { m_buckets };
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
            } else if (state == State::FILLED && m_eq(bucket.key(), key)) {
                // Found key
                break;
            }
        }
        return failure ? sentinelIndex : index;
    }

    std::pair<std::size_t, bool> probeForInsert(const K& key, const vector<Bucket>* bucketOverride = nullptr) {
        std::size_t index { getKeyIndex(key, bucketOverride) };
        // Probing for insertion, probing stops on an OPEN or TOMBSTONE bucket

        const auto& buckets { bucketOverride ? *bucketOverride : m_buckets};
        const std::size_t bucketSize { buckets.size() };
        assert(bucketSize > 0 && "bucketSize not greater than 0 in probe");

        bool failure = false;
        std::size_t tombstoneIndex { sentinelIndex };
        std::size_t iterations {};
        for (; iterations < bucketSize; ++iterations, index = (index + 1) % bucketSize) {
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
            } else if (m_eq(bucket.key(), key)) {
                assert (state == State::FILLED);
                // Key already exists, no op
                failure = true;
                break;
            }
        }

        if (tombstoneIndex != sentinelIndex) {
            // Provide the last found tombstone for insertion if found
            index = tombstoneIndex;
        }

        return std::make_pair(
            iterations < bucketSize ? index : sentinelIndex,
            !failure
            );
    }

    std::size_t probeForFilled(std::optional<std::size_t> startingIndex = std::nullopt) const {
        for (std::size_t i { startingIndex.value_or(0) }; i < m_buckets.size(); ++i) {
            if (m_buckets[i].state == State::FILLED) {
                return i;
            }
        }
        return m_buckets.size(); // end() index
    }

    template <typename vt>
    std::pair<iterator, bool> insert_impl(vt&& pair, vector<Bucket>* bucketOverride = nullptr) {
        // Rehash if necessary
        if (bucketOverride == nullptr && getLoadFactor(1) >= 0.70f) {
            rehash(m_buckets.size() * 2);

        }
        // Take the reference AFTER a potential rehash
        vector<Bucket>& buckets { bucketOverride ? *bucketOverride : m_buckets };
        std::pair<std::size_t, bool> probeReturn { probeForInsert(pair.first, bucketOverride) };

        if (probeReturn.second) {
            // We only insert if probing for a suitable bucket was successful
            auto& bucket = buckets[probeReturn.first];
            State state = bucket.state;
            if (state != State::OPEN && state != State::TOMBSTONE) {
                std::cerr << "state is: " << static_cast<int>(state) << '\n';
                assert(false && "Unreachable code reached in insert State was FILLED");
                return { end(), false };
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
        }

        HM_ASSERT_VALID();
        return { iterator{ probeReturn.first, this }, probeReturn.second };
    }

    std::size_t eraseAtIndex(std::size_t index, bool clear = false) noexcept {
        std::size_t erasedIndex { sentinelIndex };
        if (index < m_buckets.size()) {
            auto& bucket { m_buckets[index] };
            if (bucket.state == State::FILLED) {
                bucket.ptr()->~value_type();
                if (clear) {
                    // If we're clearing all elements, we set the state to OPEN
                    bucket.state = State::OPEN;
                } else {
                    bucket.state = State::TOMBSTONE;
                    ++m_tombstones;
                }

                --m_filled;
                erasedIndex = index;
            } else if (bucket.state == State::TOMBSTONE && clear) {
                bucket.state = State::OPEN;
                --m_tombstones;
            }
        }
        HM_ASSERT_VALID();
        return erasedIndex;
    }

    void destroyElements(vector<Bucket>* bucketOverride = nullptr) {
        auto& buckets { bucketOverride ? *bucketOverride : m_buckets };
        for (std::size_t i {}; i < buckets.size(); ++i) {
            Bucket& bucket { buckets[i] };
            if (bucket.state == State::FILLED) {
                bucket.ptr()->~value_type();
            }
        }
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
        if (n == 0) {
            throw std::invalid_argument("A hash_map must be initialized with a value of at least 1");
        }
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

    ~hash_map() {
        destroyElements();
    };

    ///////////////
    // Modifiers //
    ///////////////

    std::pair<iterator, bool> insert(K&& first, V&& second) {
        return insert_impl(value_type{std::move(first), std::move(second)});
    }
    std::pair<iterator, bool> insert(const K& first, const V& second) {
        return insert_impl(value_type{ first, second });
    }

    std::pair<iterator, bool> insert(const value_type& pair) {
        return insert_impl(pair);
    }

    std::pair<iterator, bool> insert(value_type&& pair) {
        return insert_impl(std::move(pair));
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

    std::size_t erase(const K& key) {
        // Returns the number of elements erased (0 or 1)
        std::size_t erasedIndex { eraseAtIndex(probeForKey(key)) };
        if (erasedIndex != sentinelIndex) {
            return 1;
        }
        return 0;
    }

    iterator erase(iterator pos) {
        // Iterator must be valid and dereferenceable
        std::size_t erasedIndex { eraseAtIndex(pos.m_currentIndex) };

        std::size_t nextFilledIndex { probeForFilled(erasedIndex + 1) };
        return { nextFilledIndex, this };
    }

    void clear() {
        for (std::size_t i {}; i < m_buckets.size(); ++i) {
            eraseAtIndex(i, true);
        }
        assert(m_tombstones == 0 && m_filled == 0);
        HM_ASSERT_VALID();
    }

    ////////////
    // Lookup //
    ////////////

    iterator find(const K& key) {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            return end();
        }
        return { index, this };
    }

    const_iterator find(const K& key) const {
        std::size_t index { probeForKey(key) };
        if (index >= m_buckets.size()) {
            return end();
        }
        return { index, this };
    }

    bool contains(const K& key) const {
        return probeForKey(key) < m_buckets.size();
    }

    std::size_t size() const noexcept {
        return m_filled;
    }

    V& operator[](const K& key) {
        auto it { find(key) };
        if (it == end()) {
            auto insertReturn { insert_impl(value_type(key, {})) };
            return insertReturn.first->second;
        }
        return it->second;
    }

    V& at(const K& key) {
        auto it { find(key) };
        if (it == end()) {
            throw std::out_of_range("The key provided was not found in the hashmap\n");
        }
        return it->second;
    }

    const V& at(const K& key) const {
        auto it { find(key) };
        if (it == end()) {
            throw std::out_of_range("The key provided was not found in the hashmap\n");
        }
        return it->second;
    }

    bool empty() const noexcept {
        return m_filled == 0;
    }

    std::size_t bucket_count() const {
        return m_buckets.size();
    }

    /////////////
    // Hashing //
    /////////////

    // Exception guarantee:
    // Strong if type is copyable or nothrow movable
    // Otherwise basic only
    void rehash(std::size_t count) {
        if (count <= bucket_count()) return;
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
            destroyElements(&newBuckets);
            throw;
        }

        auto* oldBuckets { &m_buckets };

        m_buckets = std::move(newBuckets);
        m_tombstones = 0;

        destroyElements(oldBuckets);
        HM_ASSERT_VALID();
    }

    void reserve(std::size_t count) {
        // This ensures that rehashing isn't necessary to hold `count` elements
        if (count <= bucket_count()) return; // No op
        rehash(std::ceil(count / max_load_factor()));
    }

    float load_factor() const {
        return getLoadFactor();
    }

    float max_load_factor() const {
        return maxLoadFactor;
    }

private:
    ///////////////
    // Iterators //
    ///////////////

    template <bool IsConst>
    class iterator_impl {
        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
        using owner_type = std::conditional_t<IsConst, const hash_map, hash_map>;
        using bucket_type = std::conditional_t<IsConst, const Bucket, Bucket>;

        std::size_t m_currentIndex = sentinelIndex;
        owner_type* m_owner = nullptr;

    public:
        iterator_impl() = default;

        iterator_impl(std::size_t currentIndex, owner_type* owner) noexcept
            : m_currentIndex(currentIndex)
            , m_owner(owner)
        {}

        template <bool OtherConst>
        requires(IsConst && !OtherConst)
        iterator_impl(const iterator_impl<OtherConst>& other)  noexcept {
            m_currentIndex = other.m_currentIndex;
            m_owner = other.m_owner;
        }

        reference operator*() const {
            assert(m_currentIndex != m_owner->m_buckets.size() && "Attempted to dereference an end iterator");
            bucket_type& bucket { m_owner->m_buckets[m_currentIndex] };
            assert(bucket.state == State::FILLED && "Attempted to dereference a non-FILLED iterator");
            return *bucket.ptr();
        }

        pointer operator->() const {
            assert(m_owner && "m_owner is a nullptr");
            assert(m_currentIndex != m_owner->m_buckets.size() && "Attempted to dereference an end iterator");
            bucket_type& bucket { m_owner->m_buckets[m_currentIndex] };
            assert(bucket.state == State::FILLED && "Attempted to dereference a non-FILLED iterator");
            return bucket.ptr();
        }

        iterator_impl& operator++() {
            std::size_t nextFilledIndex { m_owner->probeForFilled(m_currentIndex + 1) };
            m_currentIndex = nextFilledIndex;
            return *this;
        }

        iterator_impl operator++(int) {
            iterator_impl itCopy { *this };
            m_currentIndex = m_owner->probeForFilled(m_currentIndex + 1);
            return itCopy;
        }

        template <bool OtherConst>
        bool operator==(const iterator_impl<OtherConst>& other) const {
            return m_owner == other.m_owner && m_currentIndex == other.m_currentIndex;
        }

        template <bool OtherConst>
        bool operator!=(const iterator_impl<OtherConst>& other) const {
            return (m_owner != other.m_owner || m_currentIndex != other.m_currentIndex);
        }

        template <bool>
        friend class iterator_impl;
        friend class hash_map;
    };

public:
    iterator begin() {
        return { probeForFilled(), this };
    }

    const_iterator begin() const {
        return { probeForFilled(), this };
    }

    iterator end() {
        return { m_buckets.size(), this };
    }

    const_iterator end() const {
        return { m_buckets.size(), this };
    }

    const_iterator cbegin() const {
        return { probeForFilled(), this };
    }

    const_iterator cend() const {
        return { m_buckets.size(), this };
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
                const auto& foundIterator { find(bucket.key()) };
                assert(foundIterator != end() && "end iterator returned when attempting to find valid key");
                if (!(&foundIterator->second == &bucket.val())) {
                    assert(false && "valFound did not equal expected value");
                }
            }
        }
        assert(tombstones == m_tombstones && "Tombstone count has drifted");
        assert(filled == m_filled && "Filled count has drifted");
        assert(open == m_buckets.size() - m_filled - m_tombstones && "Open count has drifted");
        assert(open > 0 && "Open count was not greater than zero");
        assert(getLoadFactor() == static_cast<double>(filled + tombstones) / m_buckets.size() && "Load factor calculation is incorrect");
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