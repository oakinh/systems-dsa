cmake_minimum_required(VERSION 3.22.1)

project(systems-dsa VERSION 0.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(SYSTEMS_DSA_BUILD_TESTS           "Build unit tests" ON)
option(SYSTEMS_DSA_BUILD_BENCHMARKS      "Build benchmarks" ON)
option(SYSTEMS_DSA_BUILD_SCRATCH         "Build src/debug.cpp scratch executable" ON)
option(SYSTEMS_DSA_WARNINGS_AS_ERRORS    "Treat warnings as errors (your targets only)" ON)

# Let CTest/BENCH CMakeLists use BUILD_TESTING conventionally.
include(CTest)

# ------------------------------------------------------------------------------
# Target-scoped "project options" (NO global add_compile_options!)
# ------------------------------------------------------------------------------
add_library(systems_dsa_options INTERFACE)
target_compile_features(systems_dsa_options INTERFACE cxx_std_23)

if(MSVC)
    target_compile_options(systems_dsa_options INTERFACE /W4)
    if(SYSTEMS_DSA_WARNINGS_AS_ERRORS)
        target_compile_options(systems_dsa_options INTERFACE /WX)
    endif()
else()
    target_compile_options(systems_dsa_options INTERFACE -Wall -Wextra -Wpedantic)
    if(SYSTEMS_DSA_WARNINGS_AS_ERRORS)
        target_compile_options(systems_dsa_options INTERFACE -Werror)
    endif()
endif()

# ------------------------------------------------------------------------------
# Core library: header-only, but real target for consumers/tests/benches
# ------------------------------------------------------------------------------
add_library(systems_dsa INTERFACE)
add_library(systems_dsa::systems_dsa ALIAS systems_dsa)

target_link_libraries(systems_dsa INTERFACE systems_dsa_options)
target_include_directories(systems_dsa
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Explicitly list project headers that belong to the library target.
# This is the key "CLion/Nova just works" move: files are in a target.
target_sources(systems_dsa INTERFACE
        include/systems_dsa/vector.hpp
)

# ------------------------------------------------------------------------------
# clang-tidy: apply ONLY to your targets (NOT third-party)
# Presets already set CMAKE_CXX_CLANG_TIDY appropriately.
# ------------------------------------------------------------------------------
function(systems_dsa_apply_tidy target_name)
    if(CMAKE_CXX_CLANG_TIDY)
        set_property(TARGET ${target_name} PROPERTY CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
    endif()
endfunction()

# ------------------------------------------------------------------------------
# Scratchpad executable (src/debug.cpp)
# ------------------------------------------------------------------------------
if(SYSTEMS_DSA_BUILD_SCRATCH)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/debug.cpp")
        add_executable(systems_dsa_scratch src/debug.cpp)
        target_link_libraries(systems_dsa_scratch PRIVATE systems_dsa::systems_dsa)
        systems_dsa_apply_tidy(systems_dsa_scratch)
    else()
        message(STATUS "SYSTEMS_DSA_BUILD_SCRATCH=ON but src/debug.cpp not found; skipping scratch target.")
    endif()
endif()

# ------------------------------------------------------------------------------
# GoogleTest + Unit Tests
# ------------------------------------------------------------------------------
if(SYSTEMS_DSA_BUILD_TESTS AND BUILD_TESTING)
    include(FetchContent)

    # Fetch googletest; avoid installing it
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)

    # Disable clang-tidy while building third-party deps
    set(_OLD_TIDY "${CMAKE_CXX_CLANG_TIDY}")
    set(CMAKE_CXX_CLANG_TIDY "")

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    # Restore clang-tidy for your targets
    set(CMAKE_CXX_CLANG_TIDY "${_OLD_TIDY}")

    include(GoogleTest)

    # IMPORTANT: explicit file list (no GLOB) so IDE + CMake stay deterministic.
    set(SYSTEMS_DSA_TEST_SOURCES
            tests/hello_test.cpp
            tests/vector_test.cpp
    )

    # Include test helper headers too (helps CLion index them as part of the target).
    set(SYSTEMS_DSA_TEST_HEADERS
            tests/utils/lifetime_tracker.hpp
            tests/utils/throws_on_copy.hpp
    )

    add_executable(systems_dsa_tests
            ${SYSTEMS_DSA_TEST_SOURCES}
            ${SYSTEMS_DSA_TEST_HEADERS}
    )

    target_link_libraries(systems_dsa_tests
            PRIVATE
            systems_dsa::systems_dsa
            GTest::gtest_main
    )

    # If tests include headers via "utils/..." this keeps it simple.
    target_include_directories(systems_dsa_tests
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    systems_dsa_apply_tidy(systems_dsa_tests)
    gtest_discover_tests(systems_dsa_tests DISCOVERY_TIMEOUT 30)
endif()

# ------------------------------------------------------------------------------
# Benchmarks (leave benchmarks/CMakeLists.txt as-is)
# ------------------------------------------------------------------------------
if(SYSTEMS_DSA_BUILD_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
        add_subdirectory(benchmarks)
    else()
        message(STATUS "SYSTEMS_DSA_BUILD_BENCHMARKS=ON but benchmarks/ missing; skipping.")
    endif()
endif()

# ------------------------------------------------------------------------------
# clang-format helpers (optional)
# (Globbing is OK here because it does not define build sources.)
# ------------------------------------------------------------------------------
find_program(CLANG_FORMAT_EXE NAMES clang-format-19 clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE SYSTEMS_DSA_FORMAT_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/**/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/**/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/**/*.cpp"
    )

    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${SYSTEMS_DSA_FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-format on project sources"
    )

    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${SYSTEMS_DSA_FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking clang-format compliance"
    )
else()
    message(STATUS "clang-format not found; 'format' targets will not be available.")
endif()
