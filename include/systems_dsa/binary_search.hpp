#pragma once
#include <cstdio>

namespace systems_dsa {

template <typename T, typename Compare>
std::size_t lower_bound(const T* data, std::size_t n, const T& target, Compare comp) {
    const T* low { data };
    const T* high { data + n - 1 };

    if (!comp(*low, target)) return n; // No lower bound element present

    while (comp(*low, *high)) {
        const T* mid { low + (high-low) / 2 };

        if (!comp(target, *mid)) {
            low = mid;
        } else if (comp(target, *mid)) {
            high = mid - 1;
        }
    }

    return static_cast<std::size_t>(high - data);
}

template <typename T, typename Compare>
bool binary_search(const T* data, std::size_t n, const T& target, Compare comp) {
    std::size_t lb { lower_bound(data, n, target, comp) };
    const T* val { data + lb };
    return !comp(*val, target) && !comp(target, *val);
}

}
