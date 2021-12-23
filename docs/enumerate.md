# Enumerate
The enumerate function is just syntatic sugar for enumerating over integral 
ranges. The idea is for allowing easier integral loops with range-based for
loops.


# Quick Start
The simplest way to use `vecl::enumerate` is as below. 

```c++
for(auto i : vecl::enumerate(10)) // same as for(int i = 0; i < 10; ++i)
{
    // OTHER FUNCTIONS HERE...
}
```
Supply a different start value, or iterate in the negative
direction by setting end < start

```c++
// increment to 10
for(auto i : vecl::enumerate(5, 10)) // same as for(int i = 5; i < 10; ++i)
{
    // OTHER FUNCTIONS HERE...
}

// decrement to 0
for(auto i : vecl::enumerate(5, 0)) // same as for(int i = 5; i > 0; --i)
{
    // OTHER FUNCTIONS HERE...
}
```
Transform our output to be something else, using a function.

```c++
for(auto i : vecl::enumerate(10, [](auto i){ return i/2.f;}))
{
     // i = 0, 0.5, 1, 1.5
    // OTHER FUNCTIONS HERE...
}

for(auto [x, y] : vecl::enumerate(10, [](auto i){ return std::make_pair(i,i*i); }))
{
     // x = 0, 1, 2, 3, 4, 5
     // y = 0, 1, 4, 9, 16, 25
    // OTHER FUNCTIONS HERE...
}
```



