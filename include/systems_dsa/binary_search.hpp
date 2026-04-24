#pragma once
#include <cstdio>

namespace systems_dsa {

// Searches for the first element in the partitioned range which is **not** ordered before target
template <typename T, typename Compare>
std::size_t lower_bound(const T* data, std::size_t n, const T& target, Compare comp) {
    if (n == 0) return 0;

    const T* low { data };
    const T* high { data + n }; // One past end

    while (comp(low, high)) {
        const T* mid { low + (high - low) / 2 };

        if (comp(*mid, target)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return static_cast<std::size_t>(low - data);
}

template <typename T, typename Compare>
bool binary_search(const T* data, std::size_t n, const T& target, Compare comp) {
    std::size_t lb { lower_bound(data, n, target, comp) };
    const T* val { data + lb };
    return !comp(*val, target) && !comp(target, *val);
}

}
