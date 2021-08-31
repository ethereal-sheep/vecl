# Sparse Set
A Sparse Set is a contiguous container that contains strictly integral keys.

The set provides fast
1. insert,
2. remove, and
3. lookup

operations, whilst providing locality of reference.

The caveat is that we have to work with a fixed capacity; only a range of keys
are possible.

# Quick Start

`vecl::sparse_set` behaves much like `std::vector` and `std::unordered_set`.
The set has a series of constructors that mimic the `stl`.

```c++
// default construction...
vecl::sparse_set<int> a;

// ...range construction...
vecl::sparse_set<int> b(a.begin(), a.end());

// ...initializer list construction...
vecl::sparse_set<int> c{1,2,3,4};

// ...and their corresponding assignment operators
```

However, we also have to account for the fixed capacty issue. Therefore, the
set comes with capacity overloads for all its constructors as well.

```c++
// capacity constructor
vecl::sparse_set<int> a(10);

// capacity range constructor
vecl::sparse_set<int> b(a.begin(), a.end(), 10);

// capacity initializer list constructor
vecl::sparse_set<int> c({1,2,3,4}, 10);
```

The default capacity of the set is provided by the macro `VECL_SPARSE_SIZE`,
with a value of 1024. Redefinition of the macro allows the user to change the
default capacity.

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
a.erase(a.rbegin());
a.clear();

// capacity
assert(a.empty() && !a.size());

// lookup
a.insert(1);
assert(a.contains(1));
assert(a.count(1) == 1);
```



# Further Reading
