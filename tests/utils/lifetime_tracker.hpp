#pragma once
#include <cassert>
#include <ostream>

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
        assert(m_alive && "Destruction called on non-constructed object");
        ++dtorCount;
        --liveCount;
        m_alive = false;
    }

    LifetimeTracker(const LifetimeTracker& other [[maybe_unused]]) {
        assert(other.m_alive && "Copied from class is not alive.");
        m_alive = true;
        ++copyCtorCount;
        ++liveCount;
    }

    LifetimeTracker(LifetimeTracker&& other [[maybe_unused]]) noexcept {
        assert(other.m_alive && "Moved from class was not alive.");
        m_alive = true;
        ++moveCtorCount;
        ++liveCount;
    }

    LifetimeTracker& operator=(const LifetimeTracker& other) {
        if (&other == this) {
            return *this;
        }
        assert(this->m_alive && "Copy assignment target is not alive");
        assert(other.m_alive && "Copy assigned from is not alive.");
        ++copyAssignCount;
        m_alive = other.m_alive;
        return *this;
    }

    LifetimeTracker& operator=(LifetimeTracker&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        assert(this->m_alive && "Move assignment target is not alive");
        assert(other.m_alive && "Move assigned from is not alive.");
        ++moveAssignCount;
        m_alive = other.m_alive;
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