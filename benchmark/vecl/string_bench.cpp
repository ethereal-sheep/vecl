#include <string>
#include <algorithm>
#include <random>

#include <vecl/java_string.hpp>

#include <benchmark/benchmark.h>


static std::mt19937 generator;

template<size_t N, size_t L, size_t S = 20, size_t E = 126>
class large_strings
{
    char _strings[N][L];

public:
    large_strings()
    {
        std::uniform_int_distribution<> distribution(S, E);
        for (auto i = 0; i < (int)N; ++i)
        {
            for (auto j = 0; j < (int)L - 1; ++j)
            {
                _strings[i][j] = distribution(generator);
            }

            _strings[i][L - 1] = 0;
        }
    }

    auto begin() const
    {
        return std::begin(_strings);
    }

    auto end() const
    {
        return std::end(_strings);
    }

    auto operator[](size_t i) const
    {
        return _strings[i];
    }

    constexpr auto size() const
    {
        return N;
    }

};


large_strings<100, 80> strings1;
large_strings<1000, 40> strings2;
large_strings<1000, 10> strings3;
large_strings<1000, 50, 65,67> close_strings;


template<typename StringT>
static void CreateAndDump(benchmark::State& state) {
    for (auto _ : state) {
        (void)_;

        auto fn = [](auto& strings)
        {
            for (const auto* c : strings)
            {
                StringT a = c;
            }
        };

        fn(strings1);
        fn(strings2);
        fn(strings3);
        fn(close_strings);


        benchmark::ClobberMemory();
    }
}

template<typename StringT>
static void RandomCompare(benchmark::State& state) {
    for (auto _ : state) {
        (void)_;

        auto fn = [](auto& strings)
        {
            for (const auto* c : strings)
            {
                std::uniform_int_distribution<> distribution(0, strings.size() - 1);
                StringT a = c;
                a == strings[distribution(generator)];
            }
        };

        fn(strings1);
        fn(strings2);
        fn(strings3);
        fn(close_strings);

        benchmark::ClobberMemory();
    }
}

template<typename StringT>
static void ConstructThenRandomCompare(benchmark::State& state) {
    for (auto _ : state) {
        (void)_;

        auto fn = [](auto& strings)
        {
            std::vector<StringT> strings_v(strings.begin(), strings.end());
            std::uniform_int_distribution<> distribution(0, strings_v.size() - 1);
            for (auto& s : strings_v)
            {
                s == strings_v[distribution(generator)];
            }
        };

        fn(strings1);
        fn(strings2);
        fn(strings3);
        fn(close_strings);

        benchmark::ClobberMemory();
    }
}


static constexpr int MAX_SIZE = 8 << 10;


//BENCHMARK_TEMPLATE(CreateAndDump, std::string);
//BENCHMARK_TEMPLATE(CreateAndDump, vecl::java_string);
//
//BENCHMARK_TEMPLATE(RandomCompare, std::string);
//BENCHMARK_TEMPLATE(RandomCompare, vecl::java_string);

BENCHMARK_TEMPLATE(ConstructThenRandomCompare, std::string);
BENCHMARK_TEMPLATE(ConstructThenRandomCompare, vecl::java_string);

// Run the benchmark
BENCHMARK_MAIN();