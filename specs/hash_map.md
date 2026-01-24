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
template <typename T, typename U>
class HashMap {
    Status status;
    std::array<Bucket<T, U> 10> hashMap;
    int openCount = 10
    int occupiedCount = 0;
}


enum Status {
    OPEN
    FILLED
    TOMBSTONE
}

struct Bucket {
    Status status
    std::optional<T> k
    std::optional<U> v
}
```
## Invariants
- Insert will rehash at the end of the operation if non-open buckets ("FILLED" + "TOMBSTONE" count) <= 0.70 * size of array
- If a key exists, then probing from its home bucket will encounter it before encountering an OPEN bucket.
- The size of the array is always > 0 after initialization
- Probing only and always finishes on an open bucket. It also wraps around.
- When Status == FILLED, T and U are constructed and valid
- When Status == OPEN or TOMBSTONE, T and U are **not** constructed
- At the end of every operation, openCount reflects the number of Buckets with `Status` == `OPEN`
- At the end of every operation, occupiedCount reflects the number of Buckets with `Status` == `FILLED` or `TOMBSTONE`
## Growth / rehash rules
Load factor = number of elements ("FILLED" + "TOMBSTONE") / size of array
- We double the size of the array, when load factor reaches 70%
- This requires rehashing all the elements
- Rehashing converts `TOMBSTONE` buckets to `OPEN`
## Deletion strategy
- Call destructors if not trivially destructible, mark as tombstone
## Iterator & invalidation rules
- On erase, that iterator is definitely invalid, and all other iterators are guaranteed to be valid
- After a rehash, iterators are invalidated
## Error / exception behavior
Strong exception guarantee - on an exception, the container will be left in a valid, unmodified state.
- This requires that when an exception occurs, we roll back all stateful operations
- This also stipulates that T and U must have non-throwing move constructos to be moved, else they will be copied
## Non-goals (explicitly state what you are NOT supporting)
- For the first pass, we're not supporting custom hash algorithms or other probing methods. These will come later. Just using std::hash<T>