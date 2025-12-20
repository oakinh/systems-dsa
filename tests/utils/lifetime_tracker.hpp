#pragma once

class LifetimeTracker {
    bool m_alive = false;

public:
    static int liveCount;
    static int ctorCount;
    static int dtorCount;
    static int copyCtorCount;
    static int moveCtorCount;
    static int copyAssignCount;
    static int moveAssignCount;



    static void resetCounts() {
        ctorCount = 0;
        dtorCount = 0;
        copyCtorCount = 0;
        moveCtorCount = 0;
        copyAssignCount = 0;
        moveAssignCount = 0;
    }

    LifetimeTracker() {
        ++ctorCount;
        ++liveCount;
        m_alive = true;
    }

    ~LifetimeTracker() {
        ++dtorCount;
        --liveCount;
        m_alive = false;
    }

    LifetimeTracker(const LifetimeTracker& other)
    : m_alive { other.m_alive } {
        assert(m_alive && "Copied from class is not alive.\n");
        ++copyCtorCount;
        ++liveCount;
    }

    LifetimeTracker(LifetimeTracker&& other) noexcept {
        assert(other.m_alive && "Moved from class was not alive\n");
        ++moveCtorCount;
        m_alive = other.m_alive;

        other.m_alive = false;

    }

    LifetimeTracker& operator=(const LifetimeTracker& other) {
        if (&other == this) {
            return *this;
        }
        assert(other.m_alive && "Copy assigned from is not alive.\n");

        m_alive = other.m_alive;
    }

    LifetimeTracker& operator=(LifetimeTracker&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        assert(other.m_alive && "Move assigned from is not alive.\n");
    }
};