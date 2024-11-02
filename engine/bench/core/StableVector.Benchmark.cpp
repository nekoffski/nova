#include <benchmark/benchmark.h>

#include "starlight/core/utils/StableVector.hh"

#include <vector>

struct Foo {
    int x;
    float y;
};

static void emplace_std_vector(benchmark::State& state) {
    const auto n = state.range(0);
    std::vector<Foo> c;
    c.reserve(n);

    for (auto _ : state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(i);
            c.emplace_back(i, static_cast<float>(i));
        }
    }
}

static void emplace_stable_vector(benchmark::State& state) {
    const auto n = state.range(0);
    sl::StableVector<Foo> c{ static_cast<sl::u64>(n) };

    for (auto _ : state) {
        for (int i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(i);
            c.emplace(i, static_cast<float>(i));
        }
    }
}

BENCHMARK(emplace_std_vector)->RangeMultiplier(8)->Range(4, 1024 * 16);
BENCHMARK(emplace_stable_vector)->RangeMultiplier(8)->Range(4, 1024 * 16);
