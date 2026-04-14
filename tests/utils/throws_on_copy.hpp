#pragma once

class ThrowsOnCopy {
    public:
    int id {};
    bool m_alive = false;
    inline static int instanceCount = 0;
    inline static int throwOnInstance = 0;
    inline static int copyCtorCount = 0;
    inline static int dtorCount = 0;

    ThrowsOnCopy() {
        ++instanceCount;
        m_alive = true;
    };

    explicit ThrowsOnCopy(int id)
        : id { id } {
        ++instanceCount;
        m_alive = true;
    }

    ThrowsOnCopy(const ThrowsOnCopy& other) : id { other.id } {
        assert(other.m_alive);
        ++copyCtorCount;
        if (throwOnInstance != 0 && (copyCtorCount == throwOnInstance)) {
            throw std::exception();
        }
        m_alive = true;
        ++instanceCount;
    }

    ~ThrowsOnCopy() {
        assert(m_alive && "Destructor called on non-alive object");
        m_alive = false;
        ++dtorCount;
        --instanceCount;
    }

    ThrowsOnCopy(ThrowsOnCopy&& other) = delete;
    ThrowsOnCopy& operator=(ThrowsOnCopy&& other) = delete;
    ThrowsOnCopy& operator=(const ThrowsOnCopy& other) = delete;

    static void resetCounts() {
        instanceCount = 0;
        throwOnInstance = 0;
        dtorCount = 0;
        copyCtorCount = 0;
    }
};