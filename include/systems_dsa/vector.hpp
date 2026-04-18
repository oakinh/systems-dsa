#pragma once
#include <cassert>
#include <iostream>
#include <type_traits>
#include <optional>

#ifndef NDEBUG
#define VEC_ASSERT_VALID() assertValid();
#else
#define VEC_ASSERT_VALID() (void(0))
#endif


namespace systems_dsa {
    template <typename T>
    class vector {
    public:
        using reference = T&;
        using const_reference = T&;
        using size_type = std::size_t;

    // ---------------------
    // Constructors / Destructor
    // ---------------------
        // Default constructor
        vector() {
            allocate(5);
            VEC_ASSERT_VALID();
        };

        // Constructor with size
        explicit vector(size_type n) : m_capacity { n }  {
            allocate(n);
            VEC_ASSERT_VALID();
        }
        // TODO: Add constructor that takes a std::initializer_list
        vector(std::initializer_list<T> list) {
            allocate(list.size());
            for (auto element : list) {
                push_back(std::move_if_noexcept(element));
            }
        }

        // Copy constructor
        vector(const vector& other) {
            allocate(other.capacity());
            assert(m_capacity == other.capacity());
            try {
                for (size_type i {}; i < other.size(); ++i) {
                    m_data[i] = other.m_data[i];
                }
                assert(m_size == other.size());
            } catch (...) {

            }
            VEC_ASSERT_VALID();
        }

        // Copy assignment
        vector& operator=(const vector& other) {
            if (&other == this) {
                return *this;
            }

            destroyData(m_data, m_size);
            deallocate(m_data);
            allocate(other.capacity());
            assert(m_capacity == other.capacity());
            for (size_type i {}; i < other.size(); ++i) {
                m_data[i] = other.m_data[i];
            }
            m_size = other.size();
            // assert(m_size == other.size());
            VEC_ASSERT_VALID();
        }

        // Move constructor
        vector(vector&& other) noexcept {
            m_data = other.m_data;
            m_capacity = other.capacity();
            m_size = other.size();
            other.m_data = nullptr;
            other.m_size = 0;
            VEC_ASSERT_VALID();
        }

        // Move assignment
        vector& operator=(vector&& other) noexcept {
            if (&other == this) {
                return *this;
            }

            destroyData(m_data, m_size);
            deallocate(m_data);
            m_data = other.m_data;
            other.m_data = nullptr;
            m_capacity = other.capacity();
            m_size = other.size();
            other.m_size = 0;
            other.m_capacity = 0;
            VEC_ASSERT_VALID();
            return *this;
        }

        // Destructor
        ~vector() {
            destroyData(m_data, m_size);
            deallocate(m_data);
            m_size = 0;
            m_capacity = 0;
        }
    // ---------------------
    // Size & Capacity
    // ---------------------
        size_type size() const {

            return m_size;
        }
        size_type capacity() const {
            return m_capacity;
        }
        bool empty() const {
            return m_size == 0;
        }

        constexpr void reserve(size_type newCapacity) {
            if (newCapacity <= m_capacity) {
                std::cerr << "Cannot reserve less than or equal to current capacity.\n";
                return;
            }
            allocate(newCapacity);
            VEC_ASSERT_VALID();
        };

        constexpr void resize(size_type newSize) {
            if (newSize == m_size) {
                std::cerr << "Cannot resize to equal to current size.\n";
            } else if (newSize < m_size) {
                // Decrease size
                assert(m_size - newSize > 0);
                // TODO: This is probably wrong
                destroyData(m_data + newSize, m_size - newSize);
                m_size = newSize;
            } else {
                // Increase size
                size_type oldSize { m_size };
                allocate(getExpandedCapacity(newSize)); // TODO: Confirm this is intended behavior. Maybe we want capacity to equal size?
                for (size_type i { oldSize }; i < newSize; ++i) {
                    new (m_data + i) T();
                }
                m_size = newSize;
            }
            VEC_ASSERT_VALID();

        }

        void shrink_to_fit() {
            if (m_size == m_capacity) {
                std::cerr << "Capacity already matches size.\n";
                return;
            }

            // shrink_to_fit is a suggestion, we're trying to avoid waste here
            if (m_capacity > m_size * 2) {
                delete m_data;
                allocate(m_size);
            }
            VEC_ASSERT_VALID();
        };

    // ---------------------
    // Element Access
    // ---------------------

        // operator[]
        const_reference operator[](size_type index) const {
            return m_data[index];
        }
        reference operator[](size_type index) {
            return m_data[index];
        }

        // at()
        const_reference at(size_type index) const {
            if (index < m_size) {
                return m_data[index];
            }
            throw std::out_of_range("Vector index out of bounds\n");
        }
        reference at(size_type index) {
            if (index < m_size) {
                return m_data[index];
            }
            throw std::out_of_range("Vector index out of bounds\n");
        }

        // front()
        const_reference front() const {
            return m_data[0];
        }
        reference front() {
            return m_data[0];
        }

        // back()
        const_reference back() const {
            return m_data[m_size - 1];
        }
        reference back() {
            return m_data[m_size - 1];
        }

        // TODO: Revisit these
        T& operator*() { return *m_data; }
        T* operator->() { return m_data; }
        // TODO: we may not want this cast to bool, or we may want different behavior
        explicit operator bool() const { return !empty(); }

    // ---------------------
    // Pushing & popping
    // ---------------------
        void push_back(const T& value) {
            emplace_back(value);
        }

        void push_back(T&& value) {
            emplace_back(std::move(value));
        }

        template <typename... Args>
        void emplace_back(Args&&... args) {
            if (!m_data) {
                allocate(5);
            }
            if (m_size == m_capacity) {
                expand();
            }
            new (m_data + m_size) T(std::forward<Args>(args)...);
            ++m_size;
            VEC_ASSERT_VALID();
        }

        void pop_back() noexcept(std::is_nothrow_destructible_v<T>) {
            if (m_size == 0) {
                std::cerr << "There's nothing in Vector to pop\n";
                return;
            }
            if constexpr (!std::is_trivially_destructible_v<T>) {
                (m_data + m_size - 1)->~T();
            }
            --m_size;
            VEC_ASSERT_VALID();
        }

    private:
        size_type m_capacity {}; // TODO: Figure out when + how to shrink capacity after size has decreased significantly
        size_type m_size {};
        T* m_data { nullptr };

        constexpr void allocate(size_type capacity, vector* vecPtr = nullptr) {
            vector& vec = vecPtr ? *vecPtr : *this;
            void* rawMem = ::operator new(sizeof(T) * capacity, static_cast<std::align_val_t>(alignof(T)));

            if (!vec.m_data) {
                // Initial allocation
                assert(vec.m_size == 0);
                vec.m_data = static_cast<T*>(rawMem);
            } else {
                // Reallocation
                T* newData = static_cast<T*>(rawMem);
                size_type i {};
                try {
                    for (; i < vec.m_size; ++i) {
                        new (newData + i) T(std::move_if_noexcept(vec.m_data[i]));
                    }
                } catch (...) {
                    destroyData(newData, i);
                    deallocate(newData);
                    throw;
                }

                // Destroy the old data and deallocate
                destroyData(vec.m_data, vec.m_size);
                deallocate(vec.m_data);
                // Steal the new data
                vec.m_data = newData;
                vec.m_size = i;
            }

            vec.m_capacity = capacity;

            assert(vec.m_capacity >= vec.m_size && "m_capacity is greater than or equal to m_size after allocation");
        }

        void expand(std::optional<size_type> desiredCapacity = std::nullopt) {
            assert(m_size <= m_capacity && "m_size is bigger than m_capacity, there's a bug\n");
            assert(m_size == m_capacity && "m_size does not equal m_capacity when expansion was attempted\n");
            if (m_size != m_capacity) {
                std::cerr << "No need to expand, existing memory block still has room\n";
                return;
            }

            size_type newCapacity { desiredCapacity.value_or(getExpandedCapacity()) };
            assert(desiredCapacity.has_value() ? newCapacity == desiredCapacity.value() : true);
            allocate(newCapacity);
        }

        size_type getExpandedCapacity(std::optional<size_type> newSize = std::nullopt) const {
            size_type cap { newSize.value_or(m_capacity)};
            return (cap + cap / 2);
        }

        void destroyData(T* data, size_type size) noexcept(std::is_nothrow_destructible_v<T>) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                // Reverse order destruction
                for (size_type i { size }; i > 0; --i) {
                    (data + (i - 1))->~T();
                }
            }
        }
        void deallocate(T* data) noexcept {
            ::operator delete (static_cast<void*>(data), static_cast<std::align_val_t>(alignof(T)));
            data = nullptr;
        }

        void moveElements(vector& source, vector& destination) {
            assert(source.capacity() <= destination.capacity());
            void* rawMem = ::operator new(sizeof(T) * destination.capacity(), static_cast<std::align_val_t>(alignof(T)));
            T* newData = static_cast<T*>(rawMem);
            size_type i { source.size() };
            try {
                // TODO: Construction should be forward order, not reverse order
                for (; i > 0; --i) {
                    (newData + ( i - 1)) = std::move_if_noexcept(source.m_data + ( i - 1));
                }
            } catch (...) {
                destroyData(newData, i);
                deallocate(newData);
                return;
            }

            destination.m_data = newData;
        }




    #ifndef NDEBUG
        void assertValid() {
            assert(m_capacity >= m_size);
            if (m_size > 0) {
                const auto& ref1 [[maybe_unused]] { front() };
                const auto& ref2 [[maybe_unused]] { back() };
            }
        }
    #endif
    };
}