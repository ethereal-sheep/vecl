﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project ("vecl")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include sub-projects.
add_subdirectory ("test")
add_subdirectory ("benchmark")
