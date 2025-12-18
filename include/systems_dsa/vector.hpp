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
        T* m_data { nullptr }; // Is this plus m_size enough to maintain transparency? Capacity should not be a visible concept to the user
        bool isDestructible = false;

        constexpr bool allocate(int capacity) {
            if (std::is_destructible<T>()) isDestructible = true;
            // TODO: Add std::nothrow ?
            m_data = new (std::nothrow) T[capacity];
            if (!m_data) {
                std::cerr << "Failed to allocate m_data\n";
                return false;
            }
            m_capacity = capacity;
            // TODO: figure out if the m_capacity = capacity should be done here, or in the constructor's initializer list

            assert(m_capacity > m_size && "m_capacity is not greater than m_size after initial allocation\n");
            return true;
        }
        bool expand(std::optional<int> desiredCapacity = std::nullopt) noexcept {
            assert(m_size <= m_capacity && "m_size is bigger than m_capacity, there's a bug\n");
            assert(m_size == m_capacity && "m_size does not equal m_capacity when expansion was attempted\n");
            if (m_size != m_capacity) {
                std::cerr << "No need to expand, existing memory block still has room\n";
                return false;
            }
            //int newCapacity { m_capacity + ( m_capacity / 2)};
            return forceRealloc(desiredCapacity);
        }

        // forceRealloc takes no regard for current capacity vs desiredCapacity
        constexpr bool forceRealloc(std::optional<int> desiredCapacity = std::nullopt) noexcept {
            int newCapacity { desiredCapacity.value_or(m_capacity + ( m_capacity / 2)) };
            assert(desiredCapacity.has_value() ? newCapacity == desiredCapacity.value() : true);
            T* newData { new (std::nothrow) T[newCapacity] };
            if (!newData) {
                std::cerr << "Failed to allocate newData in expand\n";
                return false;
            }
            m_capacity = newCapacity;

            for (int i { 0 }; i < m_size; ++i) {
                // TODO: Figure out how to move objects that support move semantics
                newData[i] = m_data[i];
            }
            delete[] m_data;
            m_data = newData;
            return true;
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

        // Destructor
        ~vector() {
            delete[] m_data;
            m_data = nullptr;
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

            delete[] m_data;
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
            forceRealloc(newCapacity);
        };

        void shrink_to_fit() {
            if (m_size == m_capacity) {
                std::cerr << "Capacity already matches size.\n";
                return;
            }

            // shrink_to_fit is a suggestion, we're trying to avoid waste here
            if (m_capacity > m_size * 2) {
                delete[] m_data;
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
                if (!allocate(2)) {
                    std::cerr << "push_back failed, failed to allocate initial vector.\n";
                }
            }

            if (m_size == m_capacity) {
                if (!expand()) {
                    std::cerr << "push_back failed, leaving m_data in previous valid state.\n";
                    return;
                }
            }
            m_data[m_size] = value;
            ++m_size;
        }

        void push_back(const T&& value) {
            if (!m_data) {
                if (!allocate(2)) {
                    std::cerr << "push_back failed, failed to allocate initial vector.\n";
                }
            }
            if (m_size == m_capacity) {
                if (!expand()) {
                    std::cerr << "push_back failed, leaving m_data in previous valid state.\n";
                    return;
                }
            }
            // TODO: Figure out if I should handle rvalues differently than lvalues. Move?
            // I'm thinking since value is an rvalue, if it has a move assignment operator, it should use that
            // So maybe I don't have to worry about it?
            m_data[m_size] = value;
            ++m_size;
        }

        void pop_back() {
            // TODO: Test proper destruction
            if (m_size == 0) {
                std::cerr << "There's nothing in Vector to pop\n";
                return;
            }
            std::destroy_at(m_data + m_size - 1);
            --m_size;
        }
    };
}