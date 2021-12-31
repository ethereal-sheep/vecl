
cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project(vecl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


include(FetchContent)
FetchContent_Declare(
  benchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.6.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set (BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Turn off tests" FORCE)
set (BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Turn off tests" FORCE)
set (BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Turn off tests" FORCE)
FetchContent_MakeAvailable(benchmark)

get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR}/include)

function(BASIC_TEST TEST_NAME TEST_SOURCES)
	add_executable(${TEST_NAME} ${TEST_SOURCES})
if(MSVC)
	target_compile_options(${TEST_NAME} PRIVATE /W4)
else()
	target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
	target_link_libraries(${TEST_NAME} benchmark::benchmark benchmark::benchmark_main)
	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()



BASIC_TEST(_vector_bench vecl/vector_bench.cpp)