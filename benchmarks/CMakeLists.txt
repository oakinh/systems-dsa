include(FetchContent)

# --- Google Benchmark (no self-tests) ---
FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING     OFF CACHE BOOL "" FORCE)
#set(BENCHMARK_ENABLE_EXCEPTIONS  ON CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_LTO         OFF CACHE BOOL "" FORCE)

# Temporarily disable clang-tidy while building the third-party dep
set(_OLD_TIDY "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_CLANG_TIDY "")
FetchContent_MakeAvailable(googlebenchmark)
set(CMAKE_CXX_CLANG_TIDY "${_OLD_TIDY}")

# --- Your benchmarks target(s) ---
file(GLOB BENCHES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(systems_dsa_bench ${BENCHES})
target_link_libraries(systems_dsa_bench
        PRIVATE
            systems_dsa::systems_dsa
            benchmark::benchmark
)
# target_compile_features(systems_dsa_bench PRIVATE cxx_std_23)

if(CMAKE_CXX_CLANG_TIDY)
    set_property(TARGET systems_dsa_bench PROPERTY CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
endif()

# Optional: smoke test in CTest (doesn't measure perf)
if(BUILD_TESTING)
    add_test(NAME bench_smoke
            COMMAND systems_dsa_bench --benchmark_min_time=0.01 --benchmark_filter=.)
endif()
