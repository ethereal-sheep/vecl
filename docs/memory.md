# Memory
In `c++17`, the standard introduces the `std::pmr` namespace, which contains
the `std::pmr::polymorphic_allocator` and `std::pmr::memory_resource`. The 
`vecl` library extends the usages of these types, by including addtional 
memory resource types, and allowing polymorphic allocation in its containers. 

# Quick Start

## Debug Resource
There are two types of debug resources offered by `vecl`. A single-threaded
version and a multi-threaded variant. The latter uses a mutex and atomics for 
internal synchronization, but in terms of usage, behaves exactly the same as 
the former.

Using the debug resource is as simple as below:
```c++
// default construction...
vecl::memory_debug mr;

// using std::pmr::vector
std::pmr::vector<int> v(&mr);
```

Default construction of the debug resource uses the global default memory
resource by calling `std::pmr::get_default_resource()`. You may set the
upstream resource simply by passing a pointer to it in the constructor.
```c++
// using std::pmr provided resource
std::pmr::monotonic_buffer_resource upstream(1000);
vecl::memory_debug mr("Debug", nullptr, &upstream);
```

Once constructed and in use, querying data from the resource can be done by
calling one of its many member functions.

```c++
// bytes
mr.bytes_allocated();                   // total bytes allocated
mr.bytes_deallocated();                 // total bytes deallocated
mr.bytes_outstanding();                 // total bytes outstanding
mr.bytes_highwater();                   // highest number of outstanding bytes

// blocks
mr.blocks_allocated();                  // total blocks allocated
mr.blocks_outstanding();                // total blocks outstanding

// history
mr.last_allocated_num_bytes();          // bytes allocated in previous allocation
mr.last_allocated_alignment();          // alignment of previous allocation
mr.last_allocated_address();            // address of previous allocation
mr.last_deallocated_num_bytes();        // bytes allocated in previous deallocation
mr.last_deallocated_alignment();        // alignment of previous deallocation
mr.last_deallocated_address();          // address of previous deallocation
```

The debug resource also offers static functions to check for memory leaks, after
the resource goes out of scope.

```c++
vecl::memory_debug::leaked_bytes();     // number of bytes leaked
vecl::memory_debug::leaked_blocks();    // number of blocks leaked
```

For even more reporting information, the debug resource accepts a callback
function object that gets called upon any major/minor event that occurs on
allocation and deallocation. 

The library provides a default callback that writes to `std::cout`.

```c++
vecl::memory_debug mr("Debug", vecl::get_default_memory_debug_callback());
```
The user may write their own callback function by inheriting from the
`vecl::memory_debug_callback` interface. See [memory_debug_callback.hpp](..\include\vecl\memory\memory_debug_callback.hpp) for more infomation.

## Other Memory Resources
The library includes two more memory resource types that may be useful in
certain cases.

### Malloc Resource
The malloc resource uses `std::malloc`, as opposed to `new`, for memory 
allocation.

```c++
vecl::memory_debug mr("Debug", vecl::get_memory_malloc());
```

### Aligned Alloc Resource
The aligned alloc resource is useful when requiring over-aligned allocations.

```c++
vecl::memory_debug mr("Debug", vecl::get_memory_aligned_alloc());
```
## Usage
The containers in `vecl` accept a `std::pmr::polymorphic_allocator` as an 
argument in their constructors. Since the allocator can be directly constructed 
from a pointer to a `std::pmr::memory_resource`, we can simply pass in a pointer 
to the memory resource we want to use.

Some examples:
```c++
// using debug resource
vecl::memory_debug mr;
vecl::sparse_set<int> a(&mr);

// using malloc resource
vecl::sparse_map<int, std::string> b(vecl::get_malloc_resource());
```

An interesting thing to note, containers using polymorphic allocators propagate 
to their elements as well, if they use it.
```c++
vecl::memory_debug mr;
// allocates for sparse map
vecl::sparse_map<int, std::pmr::string> a(&mr);

// allocates for elements of std::pmr::string
a.push_back(1, "Hello World");
```

# Further Reading
The idea of the debug resource is inspired by a paper and talk by Pablo Halpern.
You can find the link to his github, paper, and cppcon talk below.

I also strongly suggest reading the documentation on `std::pmr` objects.

- CppCon 2017: Pablo Halpern “Allocators: The Good Parts”
   - https://www.youtube.com/watch?v=v3dz-AKOVL8 (cppcon talk)
   - https://github.com/phalpern (github)
- cppreference
    - https://en.cppreference.com/w/cpp/memory/memory_resource (Documentation on c++17 memory_resource)
    - https://en.cppreference.com/w/cpp/memory/polymorphic_allocator (Documentation on c++17 polymorphic_allocator)



