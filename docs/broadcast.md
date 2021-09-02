# Broadcast
The `vecl::broadcast` container is a smart event-handler that models a broadcast
design pattern. The container allows functions to listen to the broadcast and 
will invoke them all when triggered.


# Quick Start

Simply construct the container by specifying the type of listener function
as the template parameter.

```c++
vecl::broadcast<std::function<void()>> bc;
```

Define a listener function..
```c++
void shout_when_called() {
    std::cout << "HELLO WORLD!" << std::endl;
}
```
 ..and allow it to listen to the broadcast.
```c++
auto token = bc.listen(shout_when_called);
```

Now, when we trigger the broadcast.

```c++
bc.trigger();
```
Output:
```c++
HELLO WORLD!
```

## Token
A token obect is returned by `vecl::broadcast::listen`.
It handles the lifetime of the listener within the container;
when the token goes out of scope, the listener dies and the container 
automatically cleans it up.

```c++
vecl::broadcast<std::function<void()>> bc;
auto f1 = [](){ std::cout << "Hello from 1!" << std::endl; };
auto f2 = [](){ std::cout << "Hello from 2!" << std::endl; };
auto f3 = [](){ std::cout << "Hello from 3!" << std::endl; };

auto token1 = bc.listen(f1);
auto token2 = bc.listen(f2);
{
    auto token3 = bc.listen(f3);
} // token3 goes out of scope

bc.trigger();
```
Output:
```c++
Hello from 1!
Hello from 2!
```

The token object is also a `std::shared_ptr` of the listening
function. Therefore, the token can be dereferenced and called as well.
```c++
vecl::broadcast<std::function<void()>> bc;
auto f = [](){ std::cout << "Hello!" << std::endl; };

auto token = bc.listen(f);
(*token)();
```
Output:
```c++
Hello!
```


