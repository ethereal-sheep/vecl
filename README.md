![vecl-banner](https://user-images.githubusercontent.com/55541825/147809159-1a2373ae-35da-446a-8851-05c131398f90.png)

[![Build Status](https://github.com/ethereal-sheep/vecl/workflows/build/badge.svg)](https://github.com/ethereal-sheep/vecl/actions)
# Very Extensive Container Library
`vecl` is a header-only container library written with **modern C++**. 

It provides an extensive number of multi-purpose containers/constructs with 
many use cases. 
While "extensive" may be an overstatement, the idea is to motivate myself to 
keep improving the
library until the word may be used to describe it.

---
# Table of Contents
* [Introduction](#introduction)
   * [Goals](#goals)
   * [Supported Structures](#supported-structures)
* [Documentation](#documentation)
* [Tests](#tests)
* [License](#license)

# Introduction
## Goals
The goals of the project are to:
- provide a multi-purpose set of containers with many use cases
- widen knowledge on data structures and modern C++ techniques
- improve as a programmer

The library also uses the standard library for underlying container support 
like `std::vector` and `std::unordered_map` but creating `vecl` implementations
of `stl` containers should also be a goal in the forseeable future.

## Supported Structures

### Containers
Current container support in the library:

- Sparse Set
- Sparse Map
- Fixed Vector
- Constexpr Vector
- Small Vector
- Pipes

### Events
Current event dispatcher support in the library:

- Broadcast
- Publisher

### Memory
Current memory extensions for polymorphic allocators:

- Debug (single and multi-threaded)
- Malloc
- Aligned Alloc

### Utility
- Enumerate

### To Do
Current todo list sorted by priority:

1. [X] Deprecate Ranges/Views and replace with Pipes
   - [ ] Rethink pipes usage
2. [X] Sparse Set
   - [ ] Extend set operations for:
      - [ ] & and &= operator for union
      - [ ] | and |= operator for intersection
3. [ ] Cache-Friendly DS
   - [X] Fixed Vector
   - [X] Small Vector
   - [ ] Small Set
   - [ ] Small Map
   - [ ] SBO for existing DS
3. [ ] Lock-Free
   - [ ] SPSC Lock-Free Queue
   - [ ] SPSC Lock-Free Stack
   - [ ] MPMC Lock-Free Queue
   - [ ] MPMC Lock-Free Stack
4. [ ] Many (TEP)
5. [ ] Service Locator | Dependency Injector
6. [ ] Thread Pool
7. [ ] Adjacency List + Graph Algorithms


 # Documentation
Documentation regarding `vecl` containers can be found in the [docs](docs) 
folder. You may also use the following links to quickly access the required
documentation:

1. [Sparse Set](docs/sparse_set.md)
2. [Sparse Map](docs/sparse_map.md)
3. [Fixed Vector](docs/fixed_vector.md)
3. [Constexpr Vector](docs/constexpr_vector.md)
4. [Small Vector](docs/small_vector.md)
5. [Pipes](docs/pipes.md)
6. [Broadcast](docs/broadcast.md)
7. [Publisher](docs/publisher.md)
8. [Memory](docs/memory.md)
9. [Enumerate](docs/enumerate.md)

Internal code are also documented in a doxygen style, and can be used as a
reference when writing code.

 # Tests
Unit tests in `vecl` uses [googletest](https://github.com/google/googletest) 
library by google. You can build the tests by:

1. Cloning the repository:

    ```bash
   git clone https://github.com/ethereal-sheep/vecl.git
    ```
2. Changing directory to the repository folder:

    ```bash
   cd vecl
    ```
3. Creating a build directory and executing CMake:

    ```bash
    mkdir build
    cd build
    cmake ..
    ```
 # License
 Copyright© 2021 Cantius Chew. The library is licensed under the 
 [MIT License](LICENSE). 
 
 You can do whatever you want with the library. Just include the license when 
 you release your software, and don't take legal action with me if your stuff
 breaks.
