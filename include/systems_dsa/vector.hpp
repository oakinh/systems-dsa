#pragma once
#include <cassert>
#include <iostream>
#include <type_traits>
#include <memory>
#include <optional>

namespace systems_dsa {
    template <typename T>
    class vector {
    private:
        int m_capacity { 2 }; // TODO: Figure out when + how to shrink capacity after size has decreased significantly
        int m_size {};
        T* m_data { nullptr };

        constexpr void allocate(int capacity) {
            void* rawMem = ::operator new(sizeof(T) * capacity);

            if (!m_data) {
                // Initial allocation
                m_data = static_cast<T*>(rawMem); // TODO: This should take args for initialization
            } else {
                // Reallocation
                T* newData = static_cast<T*>(rawMem);
                int constructed {};
                try {
                    for (; constructed < m_size; ++constructed) {
                        new (newData + constructed) T(std::move_if_noexcept(m_data[constructed]));
                    }
                } catch (...) {
                    destroyData(newData, constructed);
                    throw;
                }

                destroyData(m_data, m_size);
                m_data = newData;
            }

            m_capacity = capacity;

            assert(m_capacity >= m_size && "m_capacity is greater than or equal to m_size after allocation");
        }

        void expand(std::optional<int> desiredCapacity = std::nullopt) {
            assert(m_size <= m_capacity && "m_size is bigger than m_capacity, there's a bug\n");
            assert(m_size == m_capacity && "m_size does not equal m_capacity when expansion was attempted\n");
            if (m_size != m_capacity) {
                std::cerr << "No need to expand, existing memory block still has room\n";
                return;
            }
            //int newCapacity { m_capacity + ( m_capacity / 2)};
            int newCapacity { desiredCapacity.value_or(m_capacity + ( m_capacity / 2)) };
            assert(desiredCapacity.has_value() ? newCapacity == desiredCapacity.value() : true);
            allocate(newCapacity);
        }

        void destroyData(T* data, int size) noexcept {
            if constexpr (!std::is_trivially_destructible<T>()) {
                for (int i { size }; i > 0; --i) {
                    (data + (i - 1))->~T();
                }
            }
            ::operator delete (static_cast<void*>(data));
        }
    public:
    // ---------------------
    // Constructors / Destructor
    // ---------------------
        // Default constructor
        vector() = default;

        // Constructor with size
        explicit vector(int n) : m_capacity { n }  {
            allocate(n);
        }
        // TODO: Add constructor that takes a std::initializer_list

        // Destructor
        ~vector() {
            destroyData(m_data, m_size);
        }
        // Copy constructor ?
        vector(const vector& other) = delete;

        // Copy assignment ?
        vector& operator=(const vector& other) = delete;

        // Move constructor
        vector(const vector&& vec) noexcept
            : m_data { vec.m_data } {
            m_capacity = vec.capacity();
            m_size = vec.size();
            vec.m_data = nullptr;
        }

        // Move assignment
        vector& operator=(const vector&& vec) noexcept {
            if (&vec == this) {
                return *this;
            }

            delete m_data;
            m_data = vec.m_data;
            vec.m_data = nullptr;
            m_capacity = vec.capacity();
            m_size = vec.size();

            return *this;
        }
    // ---------------------
    // Size & Capacity
    // ---------------------
        int size() const {

            return m_size;
        }
        int capacity() const {
            return m_capacity;
        }
        bool empty() const {
            return m_size == 0;
        }

      constexpr void reserve(int newCapacity) {
            if (newCapacity <= m_capacity) {
                std::cerr << "Cannot reserve less than or equal to current capacity.\n";
                return;
            }
            allocate(newCapacity);
        };

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
        };

    // ---------------------
    // Element Access
    // ---------------------
        const T& operator[](int index) const {
            return m_data[index];
        }

        T& operator[](int index) {
            return m_data[index];
        }

        T& at(int index) {
            if (index < m_size) {
                return m_data[index];
            }
            throw std::out_of_range("Vector index out of bounds\n");
            return m_data[index];
        }

        T& operator*() { return *m_data; }
        T* operator->() { return m_data; }
        // TODO: we may not want this cast to bool, or we may want different behavior
        explicit operator bool() const { return !empty(); }

    // ---------------------
    // Pushing & popping
    // ---------------------
        void push_back(const T& value) noexcept {
            if (!m_data) {
                allocate(2);
            }

            if (m_size == m_capacity) {
                expand();
            }

            new (m_data + m_size) T(value);
            ++m_size;
        }

        void push_back(T&& value) {
            if (!m_data) {
                allocate(2);
            }
            if (m_size == m_capacity) {
                expand();
            }
            new (m_data + m_size) T(std::move(value));
            ++m_size;
        }

        void pop_back() {
            // TODO: Test proper destruction
            if (m_size == 0) {
                std::cerr << "There's nothing in Vector to pop\n";
                return;
            }
            if constexpr (!std::is_trivially_destructible<T>()) {
                (m_data + m_size)->~T();
            }
            --m_size;
        }
    };
}