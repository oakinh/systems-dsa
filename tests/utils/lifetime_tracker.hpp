#pragma once

class LifetimeTracker {
public:
    static int liveCount;
    static int ctorCount;
    static int dtorCount;
    static int copyCtorCount;
    static int moveCtorCount;
    static int copyAssignCount;
    static int moveAssignCount;

    static void reset() {
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
    }

    ~LifetimeTracker() {
        ++dtorCount;
        --liveCount;
    }

    LifetimeTracker(LifetimeTracker&& other) noexcept {
        ++moveCtorCount;
    }

};