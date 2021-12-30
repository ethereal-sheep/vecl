
#include <vector>
#include <algorithm>
#include <random>

#include <vecl/fixed_vector.hpp>
#include <vecl/small_vector.hpp>
#include <vecl/enumerate.hpp>

#include <benchmark/benchmark.h>

template<typename T>
struct has_reserve {
    static constexpr bool value = false;
};


template<typename T>
struct has_reserve<std::vector<T>> {
    static constexpr bool value = true;
};
template<typename T, size_t N>
struct has_reserve<vecl::small_vector<T,N>> {
    static constexpr bool value = true;
};

template<typename T>
static constexpr bool has_reserve_v = has_reserve<T>::value;

template<std::unsigned_integral T>
static void AddValues(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        T _size = 0;
        _size += static_cast<T>(state.range(0));
    }
}

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
static void ConstructWithSizeAndElement(benchmark::State& state) {

    for (auto _ : state) {
        (void)_;
        ContainerT v(static_cast<size_t>(state.range(0)),1);
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
        if constexpr(has_reserve_v<ContainerT>)
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


template<typename ContainerT>
static void RandomSizedSwap(benchmark::State& state) {
    static std::mt19937 generator;
    std::uniform_int_distribution<std::size_t> distribution{ 1, static_cast<size_t>(state.range(0)) };
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        ContainerT v(distribution(generator));
        ContainerT x(distribution(generator));
        state.ResumeTiming();
        benchmark::DoNotOptimize(v.data());
        std::swap(v, x);
        benchmark::ClobberMemory();
    }
}


static constexpr int MAX_SIZE = 8 << 10;

//BENCHMARK_TEMPLATE(AddValues, uint32_t)->RangeMultiplier(4)->Range(8, 8 << 20);
//BENCHMARK_TEMPLATE(AddValues, size_t)->RangeMultiplier(4)->Range(8, 8 << 20);


BENCHMARK_TEMPLATE(DefaultConstruct, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(DefaultConstruct, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(DefaultConstruct, vecl::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<int>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSize, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSize, vecl::small_vector<int, 16>)->RangeMultiplier(4)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(ConstructWithSize, std::vector<std::string>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSize, vecl::fixed_vector<std::string, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSize, vecl::small_vector<std::string, 16>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(ConstructWithSizeAndElement, std::vector<int>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSizeAndElement, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(ConstructWithSizeAndElement, vecl::small_vector<int, 16>)->RangeMultiplier(4)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(EmplaceBack, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBack, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBack, vecl::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBack, vecl::small_vector<int, 16, 1.5>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<int>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::small_vector<int, 16>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::small_vector<int, 16, 1.5>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(EmplaceBackReserve, std::vector<std::string>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::fixed_vector<std::string, MAX_SIZE>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::small_vector<std::string, 4>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(EmplaceBackReserve, vecl::small_vector<std::string, 4, 1.5>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(RandomSortedInsertion, std::vector<size_t>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSortedInsertion, vecl::fixed_vector<size_t, MAX_SIZE * 2>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSortedInsertion, vecl::small_vector<size_t, 8>)->RangeMultiplier(2)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSortedInsertion, vecl::small_vector<size_t, 8, 1.5>)->RangeMultiplier(2)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(RandomSizedSwap, std::vector<int>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSizedSwap, vecl::fixed_vector<int, MAX_SIZE>)->RangeMultiplier(4)->Range(8, MAX_SIZE);
BENCHMARK_TEMPLATE(RandomSizedSwap, vecl::small_vector<int, 16>)->RangeMultiplier(4)->Range(8, MAX_SIZE);

// Run the benchmark
BENCHMARK_MAIN();

