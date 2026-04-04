# Hash Map Spec
## Goal
An unordered data structure that provides O(1) random access by keys, and can be iterated over.
It should amortize the overhead cost of hashing. This will be somewhat of a clone of std::unordered_map in its final form.
        
## Complexity targets
### Modifiers
- O(1) (amortized), except clear which is O(n)
### Lookup Operations
- Best-case + "reasonable": O(1)
- Worst-case: O(n)

## Terminology
- **bucket count:** The number of elements, regardless of `STATE`, in the underlying array.
- **size:** The number of `FILLED` elements
- **filled:** An element that has `STATE::FILLED`, and is always constructed
- **tombstone:** An element that has `STATE::TOMBSTONE`, and is not constructed. Marks an erased element that was previously filled. Available for insertion.
- **open:** An element that has `STATE::OPEN`, and is not constructed. It has never held a constructed object, and is available for insertion.
- **load factor:** The count of non-open elements ("FILLED" + "TOMBSTONE") / size of array. Used to determine when rehash and re-allocation is necessary to accommodate more elements.
- **rehash:** Allocates a new array, then rehashes and inserts all filled elements (discards tombstones).
- **capacity:** The number of elements in the underlying array, regardless of `STATE`.

## Memory layout
Contiguous array of "buckets". We use open-addressing, so one element per bucket.
This is done to achieve better cache locality, and avoid dependent memory accesses with pointer chasing.
We're using open addressing with linear probing.
- Each bucket holds one element
- Upon collision, we iterate linearly _i + 1_, _i + 2_, etc. until an open or tombstone bucket is found
- Erasing an element, marks it as a tombstone, not an open bucket.
  - This is done to ensure deleting an element does not cause probing to terminate early
- When searching for a key, the search **ends** if we find the key, or an **open bucket**. Search continues on tombstones.
```
using value_type = std::pair<K, V>

template <
        typename K, 
        typename V
        class Hasher = std::hash<K>,
        class KeyEqual = std::equal_to<K>
>
class HashMap {
    systems_dsa::vector<Bucket<K,V>> hashMap;
    size_t tombstones = 0;
    size_t size = 0 // Filled count only
    Hasher hasher;
    KeyEqual eq;
    constexpr static float max_load_factor = 0.70;
}

template <typename K, typename V>
struct Bucket {
    enum class State : uint8_t {
        OPEN
        FILLED
        TOMBSTONE
    }
    alignas(value_type) std::byte storage[sizeof(value_type)]
    State state = State::Open;
}
```
## Invariants
- Insert will rehash if it would increase non-open buckets ("FILLED" + "TOMBSTONE" count) >= 0.70 * size of array
- If a key exists, then probing from its home bucket will encounter it before encountering an OPEN bucket.
- The size of the array is always > 0 after initialization
- Probing only and always finishes either on an open bucket or the found key. It also wraps around.
  - When probing, there is guaranteed to be at least one OPEN bucket to terminate on if it failed to find the key.
  - Probing will never step > capacity
- When State == FILLED, bucket holds live `value_type`
- When State == OPEN or TOMBSTONE, there is no live object. (std::byte is uninitialized memory)
- OPEN count is always > 0
- The container's capacity = number of buckets (bucket array length)
- open = buckets.size() - m_filled - m_tombstones
- Buckets are not relocated in-place during growth; rehash allocates a new bucket array and reinserts elements.
## Supported operations
### Capacity
#### empty
- Return value: A bool. `true` if the container has no FILLED elements, `false` otherwise
- Effects: None
- Complexity: O(1)
- Exceptions / guarantee: Non-throwing
#### size
- Return value: std::size_t indicated the count of FILLED elements
- Effects: None
- Complexity: O(1)
- Exceptions / guarantee: Non-throwing
### Modifiers
#### clear
- Return value: void
- Effects: All filled elements are destructed
- Complexity: O(n)
- Exceptions / guarantee: Non-throwing
- Notes: This does not decrease the capacity of the container
#### insert
- Return value: void
- Effects: Inserts an element into the underlying array, using hashing protocol and linear probing to determine placement. Fails on an exception. Calls rehash() if load factor exceeds the threshold.
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong if type is copyable or nothrow movable, otherwise basic only
- Notes: No-ops if the key provided is already in the hash_map
#### emplace
- Return value: void
- Effects: Inserts an element into the underlying array, using hashing protocol and linear probing to determine placement. Fails on an exception. Calls rehash() if load factor exceeds the threshold.
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong if type is copyable or nothrow movable, otherwise basic only
- Notes: Currently doesn't do real piecewise emplacement. No-ops if the key provided is already in the hash_map
#### erase
- Return value: std::size_t indicating the number of erased elements
- Effects: Erases the element based on the key or index passed to it. If erased, marks the element as a tombstone.
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong exception safety guarantee
#### swap
- Not currently supported
### Lookup
#### at
- Return value: An l-value reference to the value associated with the provided key
- Effects: Performs runtime bounds checking, no other side effects.
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Throws `std::out_of_range` if provided key isn't found
#### operator[]
- Return value: An l-value reference to the value associated with the provided key
- Effects: If the key provided isn't found, it constructs a pair into the container, copy-constructing the Key, and value-initializing the mapped value. Calls rehash() if load factor exceeds the threshold.
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong if type is copyable or nothrow movable, otherwise basic only
- Notes: Key is required to be _CopyConstructible_ and the value is required to be _DefaultConstructible_.
#### find
- Return value: A pointer to the value associated with the provided key, nullptr if the key wasn't found
- Effects: No side effects
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong exception safety guarantee
#### contains
- Return value: A bool, `true` if the provided key is found within the underlying array, false otherwise
- Effects: No side effects
- Complexity: O(1) amortized best case, O(n) worst case
- Exceptions / guarantee: Strong exception safety guarantee
### Hash policy
#### reserve
- Return value: void
- Effects: Rehashes the container, increasing the bucket count such that `count` elements can be held in the hash_map **without** having to rehash again.
- Complexity: O(n)
- Exceptions / guarantee: If the type is copyable or no-throw movable, strong guarantee, otherwise basic only.
## Growth / rehash rules
Load factor = number of elements ("FILLED" + "TOMBSTONE") / size of array
- We double the size of the array, when load factor reaches 70%
  - This requires rehashing all the elements
- Rehashing converts `TOMBSTONE` buckets to `OPEN`
- Calling reserve(1000) guarantees the hashmap can hold 1000 elements without rehashing
  - This is done by allocating at least 30% more buckets than the input capacity.
- Process is:
  - Allocate a new table
  - Insert/move into it
  - Swap/commit
  - If any throw occurs, discard the new table
## Deletion strategy
- Call destructors if not trivially destructible, mark as tombstone
## Iterator & invalidation rules
- On erase, that iterator is definitely invalid, and all other iterators are guaranteed to be valid
  - Erase does not rehash
- After a rehash, iterators are invalidated
## Non-goals
- Perfect parity with std::unordered_map
- Thread-safe usage
- Custom allocator support