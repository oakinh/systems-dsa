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
- Erasing an element, marks it as a tombstone, not an open bucket
- When searching for a key, the search **ends** if we find the key, or an **open bucket**. Search continues on tombstones.

```
enum Status {
    OPEN
    FILLED
    TOMBSTONE
}

struct Bucket {
    Status status
    T k
    U v
}
```
## Invariants
- Non-open buckets ("FILLED" + "TOMBSTONE" count) <= 0.70 * size of array
- The size of the array is always > 0 after initialization
- Probing only and always finishes on a open bucket. It also wraps around.
- 
## Growth / rehash rules
Load factor = number of elements ("FILLED" + "TOMBSTONE") / size of array
- We double the size of the array, when load factor reaches 70%
- This requires rehashing all the elements
## Deletion strategy
- Call destructors if needed, mark as tombstone
## Iterator & invalidation rules
- On erase, that iterator is definitely invalid, and all other iterators are not guaranteed to be valid
- After a rehash, iterators are invalidated
## Error / exception behavior
Strong exception guarantee - on an exception, the container will be left in a valid, unmodified state.
## Non-goals (explicitly state what you are NOT supporting)
- For the first pass, we're not supporting custom hash algorithms or other probing methods. These will come later