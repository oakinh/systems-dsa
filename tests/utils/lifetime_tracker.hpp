#pragma once

class LifetimeTracker {
    bool m_alive = false;

public:
    inline static int liveCount;
    inline static int ctorCount;
    inline static int dtorCount;
    inline static int copyCtorCount;
    inline static int moveCtorCount;
    inline static int copyAssignCount;
    inline static int moveAssignCount;

    static void resetCounts() {
        liveCount = 0;
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

    LifetimeTracker(LifetimeTracker&& other) noexcept
        : m_alive { other.m_alive } {
        assert(other.m_alive && "Moved from class was not alive\n");
        other.m_alive = false;
        ++moveCtorCount;
        ++liveCount;
    }

    LifetimeTracker& operator=(const LifetimeTracker& other) {
        if (&other == this) {
            return *this;
        }
        assert(other.m_alive && "Copy assigned from is not alive.\n");
        ++copyAssignCount;
        m_alive = other.m_alive;

        return *this;
    }

    LifetimeTracker& operator=(LifetimeTracker&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        assert(other.m_alive && "Move assigned from is not alive.\n");
        ++moveAssignCount;
        m_alive = other.m_alive;

        other.m_alive = false;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& out, const LifetimeTracker& tracker) {
        out << "tracker isAlive: " << tracker.m_alive << "\n";
        out << "liveCount: " << tracker.liveCount << "\n";
        out << "ctorCount: " << tracker.ctorCount << "\n";
        out << "dtorCount: " << tracker.dtorCount << "\n";
        out << "copyCtorCount: " << tracker.copyCtorCount << "\n";
        out << "moveCtorCount: " << tracker.moveCtorCount << "\n";
        out << "copyAssignCount: " << tracker.copyAssignCount << "\n";
        out << "moveAssignCount: " << tracker.moveAssignCount << "\n";
        return out;
    }
};