cmake_minimum_required(VERSION 3.14)

project(vecl)

set(CMAKE_CXX_STANDARD 17)
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
	target_link_libraries(${TEST_NAME} gtest_main)
	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()



#BASIC_TEST(HELLO_WORLD vecl/hello_world.cpp)
#BASIC_TEST(sparse_set vecl/sparse_set.cpp)
#BASIC_TEST(memory_debug vecl/internal/memory_debug.cpp)
BASIC_TEST(sparse_map vecl/sparse_map.cpp)
