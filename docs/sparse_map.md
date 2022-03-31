# Sparse Map
A Sparse Map is a contiguous container that contains key-value pairs with 
strictly integral keys.
For a sparse representation of a hashset, see [sparse_set](sparse_set.md). 

The map provides fast
1. insert,
2. remove, and
3. lookup

operations, whilst providing locality of reference.

The caveat is that we have to work with a fixed capacity; only a range of keys
are possible.

# Quick Start

`vecl::sparse_map` behaves much like `std::vector` and `std::unordered_map`.
The map has a series of constructors that mimic the `stl`.

```c++
// default construction...
vecl::sparse_map<int, std::string> a;

// ...range construction...
vecl::sparse_map<int, std::string> b(a.begin(), a.end());

// ...initializer list construction...
vecl::sparse_map<int, std::string> c{ {1, "one"}, {2, "two"}, {3, "three"} };

// ...and their corresponding assignment operators
```

However, we also have to account for the fixed capacity issue. Therefore, the
map comes with capacity overloads for all its constructors as well.

```c++
// capacity constructor
vecl::sparse_map<int, std::string> a(10);

// capacity range constructor
vecl::sparse_map<int, std::string> b(a.begin(), a.end(), 10);

// capacity initializer list constructor
vecl::sparse_map<int, std::string> c({{1, "one"}, {2, "two"}, {3, "three"}}, 10);
```

The default capacity of the set is provided by the macro `VECL_SPARSE_SIZE`,
with a value of 1024. Redefinition of the macro allows the user to change the
default capacity.

```c++
#define VECL_SPARSE_SIZE 256 // set default sparse size to 256
```

The rest of the map's operations closely resembles that of `std::unordered_map`.

```c++
vecl::sparse_map<int, std::string> a;

// insertion
a.emplace_back(1, "one");
a.emplace_back(std::make_pair(2, "two"));
a.push_back(3, "three");
a.push_back(std::make_pair(4, "four"));
a.insert(std::make_pair(5, "five"));

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
a.emplace_back(1, "one");
assert(a.contains(1));
assert(a.count(1) == 1);

// element access
auto one = a[1];
a[2] = "two";
a.at(1) = "one more";
```


We are also able to iterate over the map via iterators, either by a traditional
for loop, or a range-based for loop.

```c++
vecl::sparse_map<int, std::string> a{ {1, "one"}, {2, "two"}, {3, "three"} };

auto for_print = [](auto& map)
{
    std::cout << "For Loop:   ";
    for(auto it = map.begin(); it != map.end(); ++it)
        std::cout << "{ " << (*it).first << ", " << (*it).second << " } ";
    std::cout << "\n";

};

auto range_print = [](auto& map)
{
    std::cout << "Range Loop: ";
    for(auto [key, value] : map)
        std::cout << "{ " << key << ", " << value << " } ";
    std::cout << "\n";
};

for_print(a);
range_print(a);

```

Output:
```
For loop:    { 1, one } { 2, two } { 3, three }
Range loop:  { 1, one } { 2, two } { 3, three }
```


# Advance
## Insertion
Due to the fixed capacity of sparse maps, we have to be careful when we insert
values into the map. As such, the `insert` and `emplace_back` functions return an
iterator-bool pair that provide information on the insert operation—similar to
what is done by `std::unordered_map`.

There are three cases that may occur:

| No. | Case | Description |
|:---:|------|-------------|
|1| Key is emplaced successfully | Boolean is true and returning iterator points to newly created key. |
|2| Key exists in set | Boolean is false and returning iterator points to existing key. |
|3| Key value > max() | Boolean is false and returning iterator == end(). |

```c++
vecl::sparse_map<int, std::string> a;

// insertion
auto [it, success] = a.emplace_back(1, "one");
auto [it2, success2] = a.emplace_back(1, "onf");
auto [it3, success3] = a.emplace_back(10);

// print lambda
auto print = [&](auto it, bool success)
{
    if(success)
        std::cout << "Success! Inserted " << (*it).second 
                  << " at " << (*it).first << std::endl; // case 1
    else
    {
        if(it != a.end())
            std::cout << "Key already exist: " << (*it).first 
                      << ", " << (*it).second << std::endl; // case 2
        else
            std::cout << "Bad Key!" << std::endl; // case 3
    }
};

// print
print(it, success);
print(it2, success2);
print(it3, success3);

```

Output:
```
Success! Inserted one at 1
Key already exist: 1, one
Bad Key!
```

## Resizing
Unlike std::vector and std::unordered_map, the user cannot add values to the set
indefinitely. As such, `vecl::sparse_map` provides a function to resize the map.

> Resizing a non-empty container is defined if and only if the maximum key value in the map is strictly less than new_size. Otherwise, the operation is undefined.[^1] 

[^1]: Might have to find a better way to handle this behaviour since the 
container is not in a valid state after the operation.

## Const Iterators
### Todo
## Sorting
While the container allows random-access of its elements, the value
of the key is important in maintaining the integrity of the container.
As such, when a key or key-value pair is returned, the key component
is either returned by value or const-reference.

Therefore, `std::sort` cannot be used with the map. As such, `vecl::sparse_map` 
provides a function to sort the set.

```c++
vecl::sparse_map<int, std::string> a{ {2, "two"}, {3, "three"}, {1, "one"} };
vecl::sparse_map<int, std::string> b{ {3, "three"}, {1, "one"}, {2, "two"} };

// print lambda
auto print = [](auto& map)
{
    for(auto [key, value] : map)
        std::cout << "{ " << key << ", " << value << " } ";
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
Before: { 2, two } { 3, three } { 1, one } 
After:  { 1, one } { 2, two } { 3, three }

Before: { 3, three } { 1, one } { 2, two } 
After:  { 3, three } { 2, two } { 1, one } 
```


# Further Reading
The idea of sparse maps are largely based on the sparse set data structure. 

For further insight on sparse sets, the following resources are great for
understanding the internals of the sparse sets and usages.

- GeeksForGeeks
    - https://www.geeksforgeeks.org/sparse-set/ (Understanding sparse set internals)
- EnTT
    - https://github.com/skypjack/entt (ECS made with sparse sets)
    - https://skypjack.github.io/2019-03-07-ecs-baf-part-2/ (Blog on ECS and sparse set usage by the creator of EnTT)

`vecl` also offers the `vecl::sparse_set` container. Documentation can be 
found [here](sparse_set.md).


