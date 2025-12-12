# Dynamic Array (Vector Clone)
## Operations
### Construction / destruction
  - `Vector()` // Default constructor
  - `Vector(size_t n)` // Constructor with size
  - `~Vector()` // Destructor 
### Element Access
- `const T& operator[](size_t index) const`
- `T& operator[](size_t index)`
- `T& at(size_t index)` runtime bounds checking
### Size & Capacity
- `size_t size() const` 
- `size_t capacity() const`
- `bool empty() const`
- `void reserve(size_t new_capacity)`
- `void shrink_to_fit()`
  - shrink_to_fit can be taken as a suggestion
### Pushing & Popping
- `void push_back(const T& value)` 
- `void pop_back()` 
### Copy / move operations
- `Vector(const Vector& other)` // Copy constructor
- `Vector& operator=(const Vector& other)` // Copy assignment

## Invariants
- Memory must be allocated _contiguously_
- `size` reflects the number of elements held
- `capacity` reflects the amount of memory allocated
- Pointer stability rules
  - After a reallocation, pointers are invalid
- `capacity` is always >= `size`
- Can be a container of references (std:ref) (aggregate relationship), or a container of values it owns

## Asymptotic Complexity Targets
- ### Element Access
- `operator[]`, `at()`
  - O(1) access, overhead for at for bounds checking
### Size & Capacity
- `size`, `capacity`, `empty`
  - O(1)
- `void reserve(size_t new_capacity)`
  - O(n) for copy/move during reallocation
  - O(1) if new_capacity <= m_capacity
- `void shrink_to_fit()`
  - O(1)
  - shrink_to_fit can be taken as a suggestion
### Pushing & Popping
- `void push_back(const T& value)`
  - Average O(1) (amortized)
  - Worst case: O(n) if it causes reallocation
- `void pop_back()`
  - O(1)
### Growth Policy
- Growth factor of 1.5
  - Calculated with capacity + (capacity / 2)

## Memory layout
- `int m_capacity`
- `int m_size`
- `T* m_data` // Pointer to first element of contiguous allocated array

## Edge Cases
- **Case:** When reserve is called with a smaller value than current capacity
  - **Action:** No op, return immediately
- **Case:** operator[] with invalid index:
  - **Action:** Throw?
- **Case:** push_back when capacity == 0
  - **Action:** Increase capacity for the needed element
- **Case:** push_back when reallocation fails
  - **Action:** Throw?
- **Case:** pop_back on empty vector
  - **Action:** No op, return immediately 


