#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>
#include <cmath>

// -----------------------------------------------------------------------------
// Example function to benchmark (placeholder – replace in real projects)
// -----------------------------------------------------------------------------
static double example_compute(double x) {
    double acc = x;
    for (int i = 0; i < 128; ++i) {
        acc = std::sin(acc) + 0.001 * i;
    }
    return acc;
}

// -----------------------------------------------------------------------------
// Simple baseline benchmark
// -----------------------------------------------------------------------------
static void BM_ExampleCompute(benchmark::State& state) {
    double x = 0.5;

    for ([[maybe_unused]] auto _ : state) {
        benchmark::DoNotOptimize(x);
        double r = example_compute(x);
        benchmark::DoNotOptimize(r);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_ExampleCompute);

// -----------------------------------------------------------------------------
// Range-based benchmark
// -----------------------------------------------------------------------------
static void BM_ExampleLoop(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    for ([[maybe_unused]] auto _ : state) {
        double acc = 0.0;
        for (int i = 0; i < n; ++i) {
            acc += example_compute(i * 0.001);
        }
        benchmark::DoNotOptimize(acc);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_ExampleLoop)
    ->RangeMultiplier(8)
    ->Range(1, 1 << 15);

// -----------------------------------------------------------------------------
// Fixture example
// -----------------------------------------------------------------------------
class ExampleFixture : public benchmark::Fixture {
public:
    std::vector<double> data;

    void SetUp(const benchmark::State& state) override {
        int n = static_cast<int>(state.range(0));
        data.resize(n);
        std::iota(data.begin(), data.end(), 0.0);
    }

    void TearDown(const benchmark::State&) override {
        data.clear();
    }
};

BENCHMARK_DEFINE_F(ExampleFixture, ProcessData)(benchmark::State& state) {
    for ([[maybe_unused]] auto _ : state) {
        double sum = 0.0;
        for (double x : data) {
            sum += example_compute(x);
        }
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK_REGISTER_F(ExampleFixture, ProcessData)
    ->RangeMultiplier(4)
    ->Range(16, 4096);

// -----------------------------------------------------------------------------
// Let Google Benchmark provide main()
// -----------------------------------------------------------------------------
BENCHMARK_MAIN();
