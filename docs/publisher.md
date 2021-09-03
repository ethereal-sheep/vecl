# Publisher
The `vecl::publisher` container is a smart event-handler that models a 
publisher-subscriber design pattern. 
The container allows functions to be subscribed to a message,
and will invoke them when the message is published.

Internally, the publisher uses `vecl::broadcast` to dispatch their messages.
See [broadcast](broadcast.md) for more information. 

# Quick Start

Simply construct the container by specifying the interface/parent type of
the messages the publisher will be sending. The library provides 
`vecl::simple_message` as the default interface if none is specified.

```c++
vecl::publisher defaultP;       // uses vecl::simple_message as interface

struct IMessage {
    ~MyMessage() {}
}

vecl::publisher<IMessage> myPublisher; // uses user-defined MyMessage as interface
```

Define a child class of interface and a calling function..
```c++
struct SimpleMessage : public IMessage {
    SimpleMessage(std::string s) : message{s} {}
    std::string message;
}

void print_simple_message(const IMessage& m) {
    auto& casted = dynamic_cast<SimpleMessage&>(m);
    std::cout << casted.message << std::endl;
}
```
 ..and subscribe it to the message via the publisher.
```c++
auto token = myPublisher.subscribe<SimpleMessage>(print_simple_message);
```

Now, when we publish the message.

```c++
myPublisher.publish<SimpleMessage>("I am a message!");
```
Output:
```c++
I am a message!
```

## Scheduling Messages
We can also schedule messages and blast them all later on in the runtime. Any
new subscribers that tune in after a message is scheduled will also receive
the message.
```c++
// first subscriber
auto token = myPublisher.subscribe<SimpleMessage>(print_simple_message);

// first scheduled message
myPublisher.schedule<SimpleMessage>("I am a scheduled message!");

/* ..some other stuff */

// second scheduled message
myPublisher.schedule<SimpleMessage>("I am a second scheduled message!");

/* ..some other stuff */

// second subscriber
auto token2 = myPublisher.subscribe<SimpleMessage>(print_simple_message);

// blast message
myPublisher.blast();
```
Output:
```c++
I am a scheduled message!
I am a scheduled message!
I am a second scheduled message!
I am a second scheduled message!
```


## Token
A token is given to the subscriber per subscription via the `subscribe` member 
function.
It handles the lifetime of the subscription within the container.
When the token goes out of scope, the subscription ends and the container 
automatically cleans it up.

```c++
auto f = [](const IMessage& m){ 
    auto& casted = dynamic_cast<SimpleMessage&>(m);
    std::cout << casted.message << std::endl; 
};

auto token1 = myPublisher.subscribe<SimpleMessage>(f);
auto token2 = myPublisher.subscribe<SimpleMessage>(f);
{
    auto token3 = myPublisher.subscribe<SimpleMessage>(f);
} // token3 goes out of scope

// token1 is destroyed
token1.reset();

myPublisher.publish<SimpleMessage>("I am a message!");
```
Output:
```c++
I am a message!
```



