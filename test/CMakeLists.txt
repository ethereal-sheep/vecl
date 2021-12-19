cmake_minimum_required(VERSION 3.14)

project(vecl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR}/include)

function(BASIC_TEST TEST_NAME TEST_SOURCES)
	add_executable(${TEST_NAME} ${TEST_SOURCES})
if(MSVC)
	target_compile_options(${TEST_NAME} PRIVATE /W4)
else()
	target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
	target_link_libraries(${TEST_NAME} gtest_main)
	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()



BASIC_TEST(_sparse_set vecl/sparse_set.cpp)
BASIC_TEST(_memory_debug vecl/memory_debug.cpp)
BASIC_TEST(_sparse_map vecl/sparse_map.cpp)
BASIC_TEST(_broadcast vecl/broadcast.cpp)
BASIC_TEST(_publisher vecl/publisher.cpp)
BASIC_TEST(_pipes vecl/pipes.cpp)
