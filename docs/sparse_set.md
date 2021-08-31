# Sparse Set
A Sparse Set is a contiguous container that contains strictly integral keys.
For a sparse representation of a hashtable, see [sparse_map](sparse_map.md). 

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
a.erase(a.begin());
a.clear();

// capacity
assert(a.empty() && !a.size());

// lookup
a.insert(1);
assert(a.contains(1));
assert(a.count(1) == 1);
```
# Advance
## Insertion
Due to the fixed capacity of sparse sets, we have to be careful when we insert
keys into the set. As such, the `insert` and `emplace_back` functions return an
iterator-bool pair that provide information on the insert operation—similar to
what is done by `std::unordered_set`.

There are three cases that may occur:

| No. | Case | Description |
|-----|------|-------------|
|1| Key is emplaced successfully | Boolean is true and returning iterator points to newly created key. |
|2| Key exists in set | Boolean is false and returning iterator points to existing key. |
|3| Key value > max() | Boolean is false and returning iterator == end(). |

```c++
vecl::sparse_set<int> a(10);

// insertion
auto [it, success] = a.emplace_back(1);
auto [it2, success2] = a.emplace_back(1);
auto [it3, success3] = a.emplace_back(10);

// print lambda
auto print = [&](auto it, bool success)
{
    if(success)
        std::cout << "Success! Inserted: " << *it << std::endl; // case 1
    else
    {
        if(it != a.end())
            std::cout << "Key already exist: " << *it << std::endl; // case 2
        else
            std::cout << "Bad Key!" << std::endl; // case 3
    }
}

// print
print(it, success);
print(it2, success2);
print(it3, success3);

```

Output:
```
Success! Inserted: 1
Key already exist: 1
Bad Key!
```

## Resizing
Unlike std::vector and std::unordered_set, the user cannot add keys to the set
indefinitely. As such, `vecl::sparse_set` provides a function to resize the set.

> Resizing a non-empty container is defined if and only if the maximum key value in the set is strictly less than new_size. Otherwise, the operation is undefined.[^1] 

[^1]: Might have to find a better way to handle this behaviour since the 
container is not in a valid state after the operation.

## Sorting
While the container allows random-access of its elements, the value
of the key is important in maintaining the integrity of the container.
As such, contents of the container are strictly read-only and are either
returned by value or by const-reference.

Therefore, `std::sort` cannot be used with the set. As such, `vecl::sparse_set` 
provides a function to sort the set.



```c++
vecl::sparse_set<int> a{5,3,4,1,2};
vecl::sparse_set<int> b{2,4,1,3,5};

// print lambda
auto print = [](auto& set)
{
    for(auto i : set)
        std::cout << i << " ";
    std::cout << "\n";
}

std::cout << "Before: ";
print(a);
// default ascending
a.sort();
std::cout << "After:  ";
print(a);

std::cout << "\nBefore: ";
print(b);
a.sort(std::greater<int>{});
std::cout << "After:  ";
print(b);

```

Output:
```
Before: 5 3 4 1 2
After:  1 2 3 4 5

Before: 2 4 1 3 5
After:  5 4 3 2 1
```

## Set Operations
[ ]: todo


# Further Reading
[ ]: todo
