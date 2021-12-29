# Fixed Vector
A Fixed Vector is a vector-like container with the caveat that it does not grow.
As such, it can also be considered an array with a vector-like interface. 

Since the buffer is always stored on stack, we can expect much faster
operations with a `vecl::fixed_vector` as compared to a `std::vector`.
	 
It is useful for when an upper bound for the size can be determined, and we
are not concerned with passing a large array around. For a similar container 
that does grow, but still storing an internal buffer on stack for small sizes,
see [small_vector](small_vector.md). 


# Quick Start

`vecl::fixed_vector` behaves much like `std::vector`.
The container has a series of constructors that mimic the `stl`.

```c++
// default construction...
vecl::fixed_vector<int, 10> a;

// fill construction...
vecl::fixed_vector<int, 10> b(10, 1);

// ...range construction...
vecl::fixed_vector<int, 10> c(b.begin(), b.end());

// ...initializer list construction...
vecl::fixed_vector<int, 10> d{ 1, 2, 3, 4 };

// ...and their corresponding assignment operators
```

The rest of the container's operations closely resembles that of 
`std::vector`.

```c++
struct MyObj {
    int _my_int;
    MyObj(int a = 0) : _my_int{a} {}
};

auto print = [](auto& fv) {
    for(auto& i : fv)
        std::cout << i._my_int << " ";
    std::cout << std::endl;
};

MyObj my_object{1};

vecl::fixed_vector<MyObj, 10> a;

// insertion
a.push_back(my_object);         // 1
a.push_back(MyObj{2});          // 1 2
a.insert(a.end(), MyObj{3});    // 1 2 3
a.emplace(a.begin(), 0);        // 0 1 2 3
a.emplace_back(4);              // 0 1 2 3 4

print(a);

// removal
a.pop_back();                   // 0 1 2 3 4
a.erase(a.begin());             // 1 2 3 4 
a.erase(a.begin(), a.end()-1);  // 4
a.clear();                      // 
```
Output:
```
0 1 2 3 4
```

We are also able to iterate over the set via iterators, either by a traditional
for loop, or a range-based for loop.

```c++
vecl::fixed_vector<int, 10> a{ 1, 2, 3, 4, 5 };

auto for_print = [](auto& fv)
{
    std::cout << "For Loop:   ";
    for(auto it = fv.begin(); it != fv.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";
};

auto range_print = [](auto& fv)
{
    std::cout << "Range Loop: ";
    for(auto i : fv)
        std::cout << i << " ";
    std::cout << "\n";
};

for_print(a);
range_print(a);

```

Output:
```
For loop:    1 2 3 4 5
Range loop:  1 2 3 4 5
```
## Overflow
Since there is no resizing of the vector when it exceeds capacity, the vector 
throws by default upon the overflow. You can set the optional Strict flag
to false to disable throwing. If disabled, no exception will be thrown upon an
overflow.
```c++
vecl::fixed_vector<int, 1> will_throw;          // Strict = true
will_throw.emplace_back();                      // ok 
will_throw.emplace_back();                      // overflow, throw

vecl::fixed_vector<int, 1, false> no_throw;     // Strict = false
no_throw.emplace_back();                        // ok 
no_throw.emplace_back();                        // overflow, ok, no insertion
```

```c++
vecl::fixed_vector<int, 1> will_throw;          // Strict = true
will_throw.emplace_back();                      // ok 
will_throw.emplace_back();                      // overflow, throw

vecl::fixed_vector<int, 1, false> no_throw;     // Strict = false
no_throw.emplace_back();                        // ok 
no_throw.emplace_back();                        // overflow, ok, no insertion
```

# Further Reading
`vecl::fixed_vector` is based on the concept of optimizing for small buffers;
if we have a container with small number of elements, we can use the stack to
store them rather than allocating memory from the heap.

For further insight on containers with small buffer optimizations, the 
following resources are great for understanding why we do small buffer 
optimizations and how to do them.

- LLVM
    - https://llvm.org/doxygen/SmallVector_8h.html (inspiration for `vecl::fixed_vector`)
- folly
    - https://github.com/facebook/folly/blob/main/folly/docs/small_vector.md (folly's own small_vector)
- CppCon
    - https://www.youtube.com/watch?v=vElZc6zSIXM (Chandler Carruth's talk on High Performance with Hybrid Data Structures)




