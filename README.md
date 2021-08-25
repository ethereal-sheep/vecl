# Very Extensive Container Library
`vecl` is a header-only container Library written with **modern C++**. 
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
of stl containers should also be a goal in the forseeable future.

## Supported Structures

### Containers
Current container support in the library.
- Sparse Set
- Sparse Map

### Memory
Current memory extensions for polymorphic allocators
- Debug (single and multi-threaded)
- Malloc
- Aligned Alloc

### To Do
Current todo list sorted by priority
1. [ ] Small Vector
2. [ ] Many (TEP)
3. [ ] Event Dispatcher | Event Queue | Callback Dispatcher | Callback Queue
4. [ ] Service Locator | Dependency Injector
5. [ ] Thread Pool
6. [ ] Adjacency List + Graph Algorithms

 
