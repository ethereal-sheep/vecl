# Set Vector
A Set Vector is a container adaptor that provide insertion-order iteration, whilst maintaining constant time lookup.


# Quick Start
By default, `vecl::set_vector` uses `std::vector` and `std::unordered_set` as underlying containers. User may provide their own containers as long as it conforms with the standard interface for both `std::vector` and `std::unordered_set`.

```c++
// using std::vector and std::unordered_set
vecl::set_vector<int> a;

// using vecl::small_vector
vecl::set_vector<int, vecl::small_vector<int, 32>> b;
```

`vecl::set_vector` behaves much like `std::vector` and `std::unordered_set`.
The set has a series of constructors that mimic the `stl`.

```c++
// default construction...
vecl::set_vector<int> a;

// ...range construction...
vecl::set_vector<int> b(a.begin(), a.end());

// ...initializer list construction...
vecl::set_vector<int> c{1,2,3,4};

// ...and their corresponding assignment operators
```

However, we also have to account for the fixed capacity issue. Therefore, the
set comes with capacity overloads for all its constructors as well.

```c++
// capacity constructor
vecl::set_vector<int> a(10);

// capacity range constructor
vecl::set_vector<int> b(a.begin(), a.end(), 10);

// capacity initializer list constructor
vecl::set_vector<int> c({1,2,3,4}, 10);
```


The rest of the set's operations closely resembles that of `std::unordered_set`.

```c++
vecl::sparse_set<int> a;

// insertion
a.emplace_back(1);
a.push_back(2);
a.insert(3);
a.insert(4);
a.insert(5);

// removal
a.pop_back();
a.remove(4);
a.erase(3);
a.erase(a.begin());
a.clear();

// capacity
assert(a.empty());
assert(a.size() == 0);

// lookup
a.insert(1);
assert(a.contains(1));
assert(a.count(1) == 1);
```

We are also able to iterate over the set via iterators, either by a traditional
for loop, or a range-based for loop.

```c++
vecl::sparse_set<int> a{1,2,3,4,5};

auto for_print = [](auto& set)
{
    std::cout << "For Loop:   ";
    for(auto it = set.begin(); it != set.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

};

auto range_print = [](auto& set)
{
    std::cout << "Range Loop: ";
    for(auto i : set)
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

# Advance
## Const Iterators
### Todo
## Set Operations
### Todo



