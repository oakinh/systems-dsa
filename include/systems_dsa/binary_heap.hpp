#pragma once
#include <functional>
#include <systems_dsa/vector.hpp>

#ifndef NDEBUG
#define BHEAP_ASSERT_VALID() assertValid();
#else
#define BHEAP_ASSERT_VALID() (void(0))
#endif

namespace systems_dsa {

template <typename T, typename Compare = std::less<T>>
class binary_heap {
public:
    // =========================
    // Member type aliases
    // =========================
    using size_type = std::size_t;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using comparator_type = Compare;

    // =========================
    // Constructors / assignment
    // =========================
    binary_heap() {
        m_data.reserve(10);
    }

    binary_heap(size_type n) {
        m_data.reserve(n);
    }

    // =========================
    // Capacity (empty, size)
    // =========================
    bool empty() noexcept {
        return m_data.size() == 0;
    }

    size_type size() noexcept {
        return m_data.size();
    }

    // =========================
    // Element access (top)
    // =========================

    reference top() const {
        return m_data[0];
    }

    // =========================
    // Modifiers (push, emplace, pop)
    // =========================
    void push(value_type val) {
        m_data.push_back(val);
        size_type insertedIndex { m_data.size() - 1 };

        if (insertedIndex == 0) return;

        for (
            size_type parentIndex { getParentIndex(insertedIndex) };
            m_comp(m_data[insertedIndex], m_data[parentIndex]);
            insertedIndex = parentIndex, parentIndex = getParentIndex(insertedIndex)
        ) {
            std::swap(m_data[insertedIndex], m_data[parentIndex]);
            if (parentIndex == 0) break;
        }
        BHEAP_ASSERT_VALID();
    }

private:
    // =========================
    // Data members
    // =========================
    systems_dsa::vector<value_type> m_data {};
    comparator_type m_comp;

    size_type getParentIndex(size_type i) const {
        assert(i > 0);
        return (i - 1) / 2;
    }

    size_type getLeftChildIndex(size_type i) const {
        assert(i > 0);
        return 2 * i + 1;
    }

    size_type getRightChildIndex(size_type i) const {
        assert(i > 0);
        return 2 * i + 2;
    }

    void assertValid() const {
        for (size_type i { 1 }; i < m_data.size(); ++i) {
            // Initialize to 1 to avoid dividing by zero in get...Index helpers
            size_type parentIndex { getParentIndex(i) };
            assert(m_comp(m_data[parentIndex], m_data[i]) && "assertValid() detected an order rule violation");
            assert(m_comp(top(), m_data[i]) && "top() providing m_data[0] was not the highest priority element");
        }
    }
};

}


