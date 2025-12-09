cmake_minimum_required(VERSION 3.14)
project(systems-dsa)

option(HPE_BUILD_TESTS "Build unit tests" ON)
option(HPE_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(HPE_ENABLE_SANITIZERS "Enable sanitizers on non-MSVC" OFF)

# ---- Warnings & sanitizers ---------------------------------------------------
if (MSVC)
    add_compile_options(/W4)
    if (HPE_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if (HPE_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
    if (HPE_ENABLE_SANITIZERS)
        add_link_options(-fsanitize=address,undefined)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()
endif()

# ---- C++ standard & extensions ----------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED CMAKE_CXX_CLANG_TIDY)
    # fallback
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Deps --------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
get_target_property(GTEST_INCLUDES GTest::gtest INTERFACE_INCLUDE_DIRECTORIES)

# Disable clang-tidy on gtest build tree.
get_property(_tidy GLOBAL PROPERTY CMAKE_CXX_CLANG_TIDY)
set_directory_properties(PROPERTIES
        COMPILE_OPTIONS ""
)

# ---- Sources -----------------------------------------------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
)

add_executable(systems-dsa ${SOURCES})
target_include_directories(systems-dsa PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Apply clang-tidy only to executable (avoids touching third-party).
if (_tidy)
    set_property(TARGET systems-dsa PROPERTY CXX_CLANG_TIDY "${_tidy}")
endif()

# ---- Tests -------------------------------------------------------------------
enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE GTest::gtest_main)
    target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/include ${GTEST_INCLUDES})

    if (_tidy)
        set_property(TARGET ${test_name} PROPERTY CXX_CLANG_TIDY "${_tidy}")
    endif()

    gtest_discover_tests(${test_name} DISCOVERY_TIMEOUT 30)
endforeach()

add_subdirectory(benchmarks)
