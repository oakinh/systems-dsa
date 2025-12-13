#pragma once
#include <cassert>
#include <exception>

namespace oakin {
    template <typename T>
    class Vector {
    private:
        int m_capacity { 2 }; // TODO: Figure out when + how to shrink capacity after size has decreased significantly
        int m_size {};
        T* m_data { nullptr }; // Is this plus m_size enough to maintain transparency? Capacity should not be a visible concept to the user

        void allocate(int capacity) {
            // TODO: Add std::nothrow ?
            m_data = new T[capacity];
            if (!m_data) {
                std::cerr << "Failed to allocate m_data\n";
                return;
            }
            m_capacity = capacity;
            // TODO: figure out if the m_size = size should be done here, or in the constructor's initializer list

            assert(m_capacity > m_size && "m_capacity is not greater than m_size after initial allocation\n");
        }
        void expand() {
            assert(m_size <= m_capacity && "m_size is bigger than m_capacity, there's a bug\n");
            assert(m_size == m_capacity && "m_size does not equal m_capacity when expansion was attempted\n");
            if (m_size != m_capacity) {
                std::cerr << "No need to expand, existing memory block still has room\n";
                return;
            }
            int newCapacity { m_capacity + ( m_capacity / 2)};
            T* newData { new T[newCapacity] };
            if (!newData) {
                std::cerr << "Failed to allocate newData in expand\n";
                return;
            }
            for (size_t i { 0 }; i < m_size; ++i) {
                // TODO: Figure out how to move objects that support move semantics
                newData[i] = m_data[i];
            }
            delete[] m_data;
            m_data = newData;

        }
    public:
    // ---------------------
    // Constructors / Destructor
    // ---------------------
        // Default constructor
        Vector() = default;

        // Constructor with size
        explicit Vector(int n) : m_capacity { n }  {
            allocate(n);
        }

        // Destructor
        ~Vector() {
            delete[] m_data;
            m_data = nullptr;
        }
    // ---------------------
    // Size & Capacity
    // ---------------------
        size_t size() const {
            assert(m_size >= 0);
            return m_size;
        }
        size_t capacity() const {
            return m_capacity;
        }
        bool empty() const {
            assert(m_size >= 0);
            return m_size == 0;
        }

        void reserve(size_t newCapacity) {
            // TODO: implement reserve
            std::cerr << ".reserve(): not implemented\n";
        };

        void shrink_to_fit() {
            // TODO: implement shrink_to_fit
            std::cerr << ".shrink_to_fit(): not implemented\n";
        };

    // ---------------------
    // Element Access
    // ---------------------
        const T& operator[](size_t index) const {
            return m_data[index];
        }

        T& operator[](size_t index) {
            return m_data[index];
        }

        T& at(size_t index) {
            if (index < m_size) {
                return m_data[index];
            }
            throw std::out_of_range("Vector index out of bounds\n");
        }

    // ---------------------
    // Pushing & popping
    // ---------------------
        void push_back(const T& value) {
            if (!m_data) allocate(2);
            if (m_size == m_capacity) {
                expand();
            }
            m_data[m_size] = value;
            ++m_size;
        }

        void push_back(const T&& value) {
            if (!m_data) allocate(2);
            if (m_size == m_capacity) {
                expand();
            }
            // TODO: Figure out if I should handle rvalues differently than lvalues. Move?
            // I'm thinking since value is an rvalue, if it has a move assignment operator, it should use that
            // So maybe I don't have to worry about it?
            m_data[m_size] = value;
            ++m_size;
        }

        void pop_back() {
            // TODO: If it's a class type, call it's destructor?
            // m_data[m_data + m_size - 1]::~T(); // Last element... TODO: but what if m_size is 0?
            // T* elementToDestroy { m_data[m_data + m_size -1] };
            // delete elementToDestroy;
            if (m_size == 0) {
                std::cerr << "There's nothing in Vector to pop\n";
            }
            --m_size;

        }
    };
}