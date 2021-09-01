# View Objects
Made available in `c++20` is the `stl` Ranges Library. However, since we are
on `c++17`, `vecl` provides simple view interfaces for ranges that may serve
as an alternative. Once the library moves forward to `c++20`, the view objects
will be deprecated.

A view object is typically an iterable object over an existing range of 
elements.

# Quick Start

The simplest way to create a view from a range is the following.

```c++
std::vector<int> v{1,2,3,4,5};

// global helper
auto view = vecl::view(v);

// or
auto view2 = vecl::view(v.begin(), v.end());
```

We can now iterate over the object and perform operations on the elements,
similar to how we can iterate over the range.

```c++
for(auto i : view)
    std::cout << i << " "; 
```

Output:
```
1 2 3 4 5
```
## Types of view objects
The library offers a few different flavours of view objects.

### Reverse
The reverse view simply calls `vecl::view(v.rbegin(), v.rend())` internally.
The range iterator must fufill the *`bidirectional_iterator`* tag.
```c++
std::vector<int> v{1,2,3,4,5};

auto view = vecl::reverse(v);

for(auto i : view)
    std::cout << i << " "; 
```

Output:
```
5 4 3 2 1
```

### Skip
The skip view is a view object that skips over certain elements that fufills
the skip predicate.
```c++
std::vector<int> v{1,2,3,4,5};

auto view = vecl::skip(v, [](auto a) { return a > 3; });

for(auto i : view)
    std::cout << i << " "; 
```

Output:
```
1 2 3
```

### Drop
The drop view is a skip view that skips a certain element.
```c++
std::vector<int> v{1,2,3,4,5};

auto view = vecl::drop(v, 3);

for(auto i : view)
    std::cout << i << " "; 
```

Output:
```
1 2 4 5
```

## Chaining
Since view objects are considered range objects, we can chain them with each
other to obtain different behaviours of iteration.


```c++
std::vector<int> v{1,2,3,4,5};

auto view = vecl::reverse(vecl::drop(v, 3));

for(auto i : view)
    std::cout << i << " "; 
```

Output:
```
5 4 2 1
```

# Further Reading
For further insight on view objects and how to make your own, the following 
resources are great for understanding what view objects are and the benefits of
using them. I also strongly recommend reading the documentation on 
`std::ranges` in `c++20`. 

- A beginner's guide to C++ Ranges and Views.
    - https://hannes.hauswedell.net/post/2019/11/30/range_intro/ 
    (Understanding the new ranges library)
- An introduction to std::string_view
    - https://www.learncpp.com/cpp-tutorial/an-introduction-to-stdstring_view/
    (Understanding std::string_view, which is available in c++17!)
- cppreference: Ranges library
    - https://en.cppreference.com/w/cpp/ranges (Documentation on c++20 ranges)

