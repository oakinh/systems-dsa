#pragma once
#include <functional>
#include <systems_dsa/vector.hpp>

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

    // =========================
    // Modifiers (push, emplace, pop)
    // =========================

private:
    // =========================
    // Data members
    // =========================
    systems_dsa::vector<value_type> m_data {};
    Compare m_comparator;
};

}


