# Binary Heap Spec
## Goal
A complete binary tree that's implemented as a contiguous array that satisfies the heap invariant ordering rule.
Ordering is done according to the Comparator template argument passed in.

## Terminology
- size_type: an alias for std::size_t
- value_type: The value type of the elements held, T
- parent: For a given node at index i, the parent is: (i - 1) / 2
- left child: For a given node at index i, the left child is: 2 * i + 1
- right child: For a given node at index i, the right child is: 2 * i + 2
- Higher priority: the element that does NOT come before others according to Compare

## Memory layout

```
template <T, Compare = std::less<T>>
class binary_heap {
    systems_dsa::vector<T> m_data {};
    Compare m_comparator;
}
```
## Invariants
- Structure property:
    - Every level in the tree is filled, with the exception of the last level possibly not being filled.
      - Elements occupy indices [0, size()], no uninitialized or inactive slots exist within this range
    - The last level will be filled left → right
- Order property:
    - For every non-root node `i`, the parent is not lower priority than node i
    - After any given operation, the container is ordered using the comparator according to the heap ordering property.
        - Ex: with std::less, index 0 will be the largest element. Each parent >= it's children.
        - Notably, only the parent vs child ordering is guaranteed, **not** global ordering.
        - The root (index 0), must always hold the highest-priority element according to Compare
          - This is the element that comes **last** in ordering according to Compare.
- m_data.size() == the number of heap elements
- top() is m_data[0] when non-empty

## Supported operations
### Capacity
#### empty
- Returns a bool, specifically `true` if the container holds no elements, `false` otherwise
- Complexity: O(1)
- Exception safety: Non-throwing
#### size
- Returns a size_type of the number of elements held in the container
- Complexity: O(1)
- Exception safety: Non-throwing
### Modifiers
#### push
- Returns void
- Appends the given element to the end of the heap and orders the container according to the comparator and order property.
  - Ordering continually checks the inserted element against its parent until the order property is achieved.
- Duplicates are allowed
- Complexity: O(log n). May cause container reallocation, spiking latency.
- Exception safety: 
  - Strong guarantee if T is nothrow move-constructible or copyable, and Compare does not throw
  - Otherwise basic guarantee
#### emplace
- Returns void
- Constructs the element in-place at the end of the heap and order the container according to the comparator and order property.
  - Ordering continually checks the inserted element against its parent until the order property is achieved.
- Complexity: O(log n). May cause container reallocation, spiking latency.
- Exception safety:
    - Strong guarantee if T is nothrow move-constructible or copyable, and Compare does not throw
    - Otherwise basic guarantee
#### pop
- Returns void
- Effect: Removes the element at index 0 (the element that would have been returned by top()) and orders the container
  - Moves the last element to index 0, and then bubbles down, comparing the priority child continually, until the order property is satisfied.
- Requires: `!empty()`
- Complexity: O(log n)
- Exception safety: 
  - Non-throwing if T is nothrow move-constructible and Compare is non-throwing
  - Otherwise, basic guarantee
### Lookup
#### top
- Returns a reference to the top element (index 0)
- Requires: `!empty()`
- Complexity: O(1)
- Exception safety: Non-throwing

## Notes:
- The structure is not stable
- Equal-priority elements will come out of the container in any relative order
- Growth follows underlying vector strategy
- Reallocation invalidates references, pointers, and iterators

## Non-goals
- STL templated Container feature parity
- Iterator support
