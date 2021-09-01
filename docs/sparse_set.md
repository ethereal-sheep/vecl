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

```c++
#define VECL_SPARSE_SIZE 256 // set default sparse size to 256
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
## Insertion
Due to the fixed capacity of sparse sets, we have to be careful when we insert
keys into the set. As such, the `insert` and `emplace_back` functions return an
iterator-bool pair that provide information on the insert operation—similar to
what is done by `std::unordered_set`.

There are three cases that may occur:

| No. | Case | Description |
|:---:|------|-------------|
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
};

std::cout << "Before: ";
print(a);
a.sort(); // default ascending
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
A common use case when using sets are performing set operations union, 
intersection and equality. The `stl` does provide some algorithms to perform
such operations, but `vecl::sparse_set` provides them explicitly as member
functions.

### Equality
There are two functions in the set that perform equality checking. The common
`operator==()`, and the `set_equal` function. The latter performs existence
checking only, while the former performs ordering check as well.

`operator!=()` is available as well.

```c++
vecl::sparse_set<int> a{5,3,4,1,2};
vecl::sparse_set<int> b{2,4,1,3,5};
vecl::sparse_set<int> c{2,4,1,3,5};

assert(a.set_equal(b)); // only existence check
assert(vecl::set_equal(a,b,c)); // global set_equal operation for multiple sets

assert(a != b)); // existence + order check
assert(b == c));
```

### Union
Like the `stl`, the function to perform the union operation between two sets
is called `merge`.

Might overload `operator&()` and `operator&=()` for union operators in the 
future.
```c++
vecl::sparse_set<int> a{1,2,3,4,5};
vecl::sparse_set<int> b{6,7,8,9,0};

// print lambda
auto print = [](auto& set)
{
    for(auto i : set)
        std::cout << i << " ";
    std::cout << "\n";
};

std::cout << "a: ";
print(a);
std::cout << "b: ";
print(b);

a.merge(b); // a = a u b, merge b into a
std::cout << "\na: ";
print(a);
```
Output:
```
a: 1 2 3 4 5
b: 6 7 8 9 0

a: 1 2 3 4 5 6 7 8 9 0
```

### Intersect
The `stl` contains an algorithm called `std::set_intersection` but its usage
is slightly different as it requires two sorted ranges as inputs. The set 
provides the `intersect` function which is slightly more similar to the 
conventional mathematical operation.

Might overload `operator|()` and `operator|=()` for intersect operators in the 
future.
```c++
vecl::sparse_set<int> a{1,2,3,4,5};
vecl::sparse_set<int> b{1,2,3,9,0};

// print lambda
auto print = [](auto& set)
{
    for(auto i : set)
        std::cout << i << " ";
    std::cout << "\n";
};

std::cout << "a: ";
print(a);
std::cout << "b: ";
print(b);

a.intersect(b); // a = a n b, removes any key in a not contained in b
std::cout << "\na: ";
print(a);
```
Output:
```
a: 1 2 3 4 5
b: 1 2 3 9 0

a: 1 2 3
```

# Further Reading
For further insight on sparse sets, the following resources are great for
understanding the internals of the sparse sets and usages.

- GeeksForGeeks
    - https://www.geeksforgeeks.org/sparse-set/ (Understanding sparse set internals)
- EnTT
    - https://github.com/skypjack/entt (ECS made with sparse sets)
    - https://skypjack.github.io/2019-03-07-ecs-baf-part-2/ (Blog on ECS and sparse set usage by the creator of EnTT)


