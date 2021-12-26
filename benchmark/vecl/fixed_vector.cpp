
#include <vector>
#include <algorithm>
#include <random>

#include <vecl/fixed_vector.hpp>
#include <vecl/enumerate.hpp>

#include <benchmark/benchmark.h>

template<typename T>
struct is_std_vector {
    static constexpr bool value = false;
};


template<typename T>
struct is_std_vector<std::vector<T>> {
    static constexpr bool value = true;
};

template<typename T>
static constexpr bool is_std_vector_v = is_std_vector<T>::value;


template<typename ContainerT>
static void ConstructWithSize(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        ContainerT v(static_cast<size_t>(state.range(0)));
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void DefaultConstruct(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        ContainerT v;
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void EmplaceBack(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v;
        state.ResumeTiming();
        for (int j = 0; j < state.range(0); ++j)
            v.emplace_back();
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void EmplaceBackReserve(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v;
        state.ResumeTiming();
        if constexpr(is_std_vector_v<ContainerT>)
            v.reserve(static_cast<size_t>(state.range(0)));
        for (int j = 0; j < state.range(0); ++j)
            v.emplace_back();
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}

template<typename ContainerT>
static void RandomSortedInsertion(benchmark::State& state) {
    static std::mt19937 generator;
    static std::uniform_int_distribution<std::size_t> distribution;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v(static_cast<size_t>(state.range(0)));
        state.ResumeTiming();
        benchmark::DoNotOptimize(v.data());
        for (std::size_t i = 0; i < static_cast<size_t>(state.range(0)); ++i) {
            auto val = distribution(generator);
            v.insert(std::lower_bound(v.begin(), v.end(), val), val);
        }
        benchmark::ClobberMemory();
    }
}

static constexpr int MAX_SIZE = 8 << 10;


//BENCHMARK_TEMPLATE(DefaultConstruct, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(DefaultConstruct, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//
//BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(ConstructWithSize, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//
//BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<std::string>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(ConstructWithSize, vecl::fixed_vector<std::string, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//
//BENCHMARK_TEMPLATE(EmplaceBack, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(EmplaceBack, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//
//BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//
//BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<std::string>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::fixed_vector<std::string, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(RandomSortedInsertion, std::vector<size_t>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSortedInsertion, vecl::fixed_vector<size_t, MAX_SIZE*2>)->RangeMultiplier(2)->Range(8, MAX_SIZE);


// Run the benchmark
BENCHMARK_MAIN();

