#pragma once

class ThrowsOnCopy {
    public:
    int id {};
    inline static int instanceCount = 0;
    inline static int throwOnInstance = 0;
    inline static int copyCtorCount = 0;
    inline static int dtorCount = 0;

    ThrowsOnCopy() = default;

    explicit ThrowsOnCopy(int id)
        : id { id } {
        ++instanceCount;
    }

    ThrowsOnCopy(const ThrowsOnCopy& other) : id { other.id } {
        if (throwOnInstance != 0 && (copyCtorCount == throwOnInstance)) {
            throw std::exception();
        }
        ++copyCtorCount;
    }
    ~ThrowsOnCopy() { ++dtorCount; }

    ThrowsOnCopy(ThrowsOnCopy&& other) = delete;
    ThrowsOnCopy& operator=(ThrowsOnCopy&& other) = delete;

    static void resetCounts() {
        instanceCount = 0;
        throwOnInstance = 0;
        dtorCount = 0;
        copyCtorCount = 0;
    }
};