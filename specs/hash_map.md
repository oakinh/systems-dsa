# Hash Map Spec
## Goal
An unordered data structure that provides O(1) random access by keys, and can be iterated over.
It should amortize the overhead cost of hashing. This will be somewhat of a clone of std::unordered_map in its final form.

## Supported operations
### Capacity
- empty
- size
### Modifiers
- clear
- insert
- emplace
- erase
- swap
### Lookup
- at
- operator[]
- find
- contains
### Bucket interface
- begin, cbegin
- end, cend

### Hash policy
- reserve
        
## Complexity targets
### Modifiers
- O(1) (amortized), except clear which is O(n)
### Lookup Operations
- Best-case + "reasonable": O(1)
- Worst-case: O(n)

## Memory layout
Contiguous array of "buckets". We'll use open-addressing for the first pass, so one element per bucket.
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
- capacity = number of buckets (bucket array length)
- open = capacity - size - tombstones
- Buckets are not relocated in-place during growth; rehash allocates a new bucket array and reinserts elements.
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
## Error / exception behavior
Strong exception guarantee for rehash by build-new-then-swap - on an exception, the container will be left in a valid, unmodified state.
Other ops have a basic exception guarantee
## Non-goals (explicitly state what you are NOT supporting)