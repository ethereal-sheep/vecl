# Small Vector
A Small Vector is dynamic array with small buffer optimizations; i.e. for 
small sizes, the elements are stored on the stack instead of the heap. Only
when the stack buffer is full do we then 'spill' on to the heap.
	 
For a similar container that stores elements on the stack but does not grow, see [fixed_vector](fixed_vector.md). 


# Quick Start

`vecl::small_vector` behaves much like `std::vector`.
The container has a series of constructors that mimic the `stl`.

```c++
// default construction...
vecl::small_vector<int, 10> a;

// fill construction...
vecl::small_vector<int, 10> b(10, 1);

// ...range construction...
vecl::small_vector<int, 10> c(b.begin(), b.end());

// ...initializer list construction...
vecl::small_vector<int, 10> d{ 1, 2, 3, 4 };

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

vecl::small_vector<MyObj, 10> a;

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
vecl::small_vector<int, 10> a{ 1, 2, 3, 4, 5 };

auto for_print = [](auto& sv)
{
    std::cout << "For Loop:   ";
    for(auto it = sv.begin(); it != sv.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";
};

auto range_print = [](auto& sv)
{
    std::cout << "Range Loop: ";
    for(auto i : sv)
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
## Small Buffer
Since the container implements small buffer optimization, elements will
be stored on stack if the size of the container is small. An example can be
seen below.
```c++
vecl::small_vector<int, 1> sv;
sv.emplace_back();              // on stack
sv.emplace_back();              // on heap
```

## Advance
### Reference Interface
At interface boundaries, using a regular reference to 
`vecl::small_vector<T,N>` may not be a wise idea. We have to either specialize 
for each N or create templates to do it for us. This may result in unneccessary 
code duplication.

Thus, `vecl` provides a type-erased object for `vecl::small_vector` references.
```c++
void foo(vecl::small_vector<int, 16>& ref) 
{
    // ok, but only accepts vecl::small_vector<int, 16>
}
template<size_t N>
void goo(vecl::small_vector<int, N>& ref) 
{
    // ok, but synthesizes unneccessary code
}   
void hoo(vecl::small_vector_ref<int>& ref) 
{
    // ok, accepts vecl::small_vector<int, N>, for any N
} 
```

### Allocator Awareness
The current `small_vector` is not allocator aware, and implementation of such
is a work in progress.

# Further Reading
For further insight on containers with small buffer optimizations, the 
following resources are great for understanding why we do small buffer 
optimizations and how to do them.

- LLVM
    - https://llvm.org/doxygen/SmallVector_8h.html (inspiration for `vecl::small_vector`)
- folly
    - https://github.com/facebook/folly/blob/main/folly/docs/small_vector.md (folly's own small_vector)
- CppCon
    - https://www.youtube.com/watch?v=vElZc6zSIXM (Chandler Carruth's talk on High Performance with Hybrid Data Structures)




