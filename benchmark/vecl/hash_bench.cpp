
#include <vector>
#include <algorithm>
#include <random>

#include <vecl/simple_set.hpp>
#include <vecl/robin_set.hpp>
#include <unordered_set>

#include <benchmark/benchmark.h>

#define MAX_MAX 1000001

template<typename T>
struct has_reserve {
    static constexpr bool value = false;
};



size_t random_array[MAX_MAX];


template<typename ContainerT, typename V>
static void Inserts(benchmark::State& state) {
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
                v.insert(std::to_string(i));
            else
                v.insert((V)i);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}

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
                v.insert(std::to_string(random_array[i]));
            else
                v.insert((V)random_array[i]);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}



template<typename ContainerT, typename V>
static void Lookups(benchmark::State& state) {
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
                (void)v.contains(std::to_string(i));
            else
                (void)v.contains((V)i);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0));
    }
}

template<typename ContainerT, typename V>
static void RandomLookups(benchmark::State& state) {
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
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
                v.contains(std::to_string(random_array[i]));
            else
                v.contains((V)random_array[i]);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0) / 5);
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
    for (auto _ : state) {
        (void)_;
        state.PauseTiming();
        auto N = static_cast<size_t>(state.range(0));
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
                v.erase(std::to_string(random_array[i]));
            else
                v.erase((V)random_array[i]);
        }


        benchmark::ClobberMemory();
        state.SetComplexityN(state.range(0) / 5);
    }
}

//template<typename ContainerT, typename V>
//static void RandomOperations(benchmark::State& state) {
//    for (auto _ : state) {
//        (void)_;
//        state.PauseTiming();
//        auto N = static_cast<size_t>(state.range(0));
//        ContainerT v;
//        state.ResumeTiming();
//
//        for (auto i = 0; i < N; ++i)
//        {
//            v.insert((V)distribution(generator));
//        }
//
//        benchmark::ClobberMemory();
//        state.SetComplexityN(state.range(0));
//    }
//}


void init_random_array(benchmark::State& state)
{
    for (auto _ : state) {
        (void)_;

        static std::mt19937 generator;
        std::uniform_int_distribution<std::size_t> distribution{ 1, MAX_MAX };
        for (auto i = 0; i < MAX_MAX; ++i)
            random_array[i] = distribution(generator);

        benchmark::ClobberMemory();
        state.SetComplexityN(MAX_MAX);
    }
}



static constexpr int MAX_SIZE = (int)1e5;
static constexpr int MIN_SIZE = 10;
static constexpr int STEP = 10;

BENCHMARK(init_random_array)->Range(1, 1)->Complexity(benchmark::oN);


//BENCHMARK_TEMPLATE(Inserts, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Inserts, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Inserts, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
//BENCHMARK_TEMPLATE(Inserts, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Inserts, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Inserts, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
//
//BENCHMARK_TEMPLATE(RandomInserts, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomInserts, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomInserts, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
//BENCHMARK_TEMPLATE(RandomInserts, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomInserts, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomInserts, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
//
//BENCHMARK_TEMPLATE(Lookups, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Lookups, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Lookups, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(Lookups, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Lookups, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Lookups, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);


//BENCHMARK_TEMPLATE(RandomLookups, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomLookups, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomLookups, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(RandomLookups, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomLookups, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomLookups, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);


//BENCHMARK_TEMPLATE(Erases, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Erases, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(Erases, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
BENCHMARK_TEMPLATE(Erases, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Erases, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(Erases, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
//
//BENCHMARK_TEMPLATE(RandomErases, vecl::robin_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomErases, vecl::simple_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//BENCHMARK_TEMPLATE(RandomErases, std::unordered_set<int>, int)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
//
BENCHMARK_TEMPLATE(RandomErases, vecl::robin_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomErases, vecl::simple_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);
BENCHMARK_TEMPLATE(RandomErases, std::unordered_set<std::string>, std::string)->RangeMultiplier(STEP)->Range(MIN_SIZE, MAX_SIZE)->Complexity(benchmark::oN);

// Run the benchmark
BENCHMARK_MAIN();

