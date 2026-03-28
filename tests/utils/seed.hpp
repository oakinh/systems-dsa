#pragma once

#include <chrono>
#include <cstdint>
#include <random>
#include <iostream>

std::uint64_t getSeed(const char* envVar) {
    const char* env = std::getenv(envVar);

    if (env) {
        try {
            return std::stoull(env);
        } catch (...) {
            std::cerr << "Invalid seed: " << envVar << " - using default\n";
        }
    }
    std::random_device rd;
    std::uint64_t seed { (static_cast<std::uint64_t>(rd()) << 32) ^
        std::chrono::high_resolution_clock::now().time_since_epoch().count() };

    std::cout << envVar << "=" << seed << '\n';
    return seed;
}
