
#include <vector>
#include <algorithm>
#include <random>

#include <vecl/simple_set.hpp>
#include <unordered_set>

#include <benchmark/benchmark.h>

template<typename T>
struct has_reserve {
    static constexpr bool value = false;
};


template<typename ContainerT, typename V>
static void RandomInserts(benchmark::State& state) {
    static std::mt19937 generator;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
        std::uniform_int_distribution<std::size_t> distribution{ 1, N };
        ContainerT v;
        state.ResumeTiming();

        for (auto i = 0; i < N; ++i)
        {
            if constexpr (std::is_same_v<V, std::string>)
                v.insert(std::to_string(distribution(generator)));
            else
                v.insert((V)distribution(generator));
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}
template<typename ContainerT, typename V>
static void Erases(benchmark::State& state) {
    static std::mt19937 generator;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
        std::uniform_int_distribution<std::size_t> distribution{ 1, N };
        ContainerT v;

        for (auto i = 0; i < N; ++i)
        {
            if constexpr (std::is_same_v<V, std::string>)
                v.insert(std::to_string(i));
            else
                v.insert((V)i);
        }

        state.ResumeTiming();

        for (auto i = 0; i < N; ++i)
        {
            if constexpr (std::is_same_v<V, std::string>)
                v.erase(std::to_string(i));
            else
                v.erase((V)i);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}

template<typename ContainerT, typename V>
static void RandomErases(benchmark::State& state) {
    static std::mt19937 generator;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
        std::uniform_int_distribution<std::size_t> distribution{ 1, N };
        ContainerT v;

        for (auto i = 0; i < N; ++i)
        {
            if constexpr (std::is_same_v<V, std::string>)
                v.insert(std::to_string(i));
            else
                v.insert((V)i);
        }

        state.ResumeTiming();

        for (auto i = 0; i < N / 5; ++i)
        {
            if constexpr (std::is_same_v<V, std::string>)
                v.erase(std::to_string(distribution(generator)));
            else
                v.erase((V)distribution(generator));
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0) / 5);
    }
}

template<typename ContainerT, typename V>
static void RandomOperations(benchmark::State& state) {
    static std::mt19937 generator;
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
        std::uniform_int_distribution<std::size_t> distribution{ 1, N };
        ContainerT v;
        state.ResumeTiming();

        for (auto i = 0; i < N; ++i)
        {
            v.insert((V)distribution(generator));
        }

        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}


static constexpr int MAX_SIZE = 8 << 13;

//BENCHMARK_TEMPLATE(RandomInserts, std::unordered_set<int>, int)->RangeMultiplier(4)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(RandomInserts, vecl::simple_set<int>, int)->RangeMultiplier(4)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(RandomInserts, std::unordered_set<std::string>, std::string)->RangeMultiplier(4)->Range(8, MAX_SIZE);
//BENCHMARK_TEMPLATE(RandomInserts, vecl::simple_set<std::string>, std::string)->RangeMultiplier(4)->Range(8, MAX_SIZE);

BENCHMARK_TEMPLATE(Erases, vecl::simple_set<int>, int)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Erases, std::unordered_set<int>, int)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Erases, vecl::simple_set<std::string>, std::string)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Erases, std::unordered_set<std::string>, std::string)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);


BENCHMARK_TEMPLATE(RandomErases, vecl::simple_set<int>, int)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomErases, std::unordered_set<int>, int)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomErases, vecl::simple_set<std::string>, std::string)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomErases, std::unordered_set<std::string>, std::string)->RangeMultiplier(8)->Range(8, MAX_SIZE)->Complexity(benchmark::oN);

// Run the benchmark
BENCHMARK_MAIN();

