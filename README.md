# Summary

"cpptask" is an open source library which implements load balanced task scheduler for developing parallel C++ programs.

The main advantages are:

1. It was made to be **similar to native C++** `async` interfaces as much as possible.
2. You can specify **number of threads**, library will not create new ones.
3. "Work stealing" technique is used for **load balancing**.
4. You can use special type of synchronization primitives and special lock type to guarantee **workload of thread during lock** waiting operation.
5. Library implemented as **headers only** library.

# Features

1. **Task management**
 * ***future*** class
 * ***async*** function

 These functionality copies standard C++ signatures, methods and behaviour except `shared_future` class.

 Usage example:

 ```cpp
 #inlcude <cpptask/cpptask.h>
 #include <chrono>
 #include <cassert>

 int main()
 {
    cpptask::initializer init(4); //initialize library with 4 threads
    auto f = cpptask::async(std::launch::async, []()
    {
        //do something
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 5;
    });

    auto x = f.get();
    assert(5 == x);
 }
 ```

2. **Synchronization**
 * ***mutex*** class - Special synchronization primitive which allows to workload thread during waiting operation. should be used only in pair with `process_lock` type, signatures identical to `std::mutex`.

    Usage example:

    ```cpp
    ...
    cpptask::mutex guard;
    ...
    {
      std::unique_lock<cpptask::mutex> lock(gurad); //totaly block thread
    }
    ...
    {
      cpptask::process_lock<cpptask::mutex> lock(gurad); //will process other tasks during lock
    }
    ...
    ```
 * ***event*** class - Event synchronization abstraction.

    Usage example:

    ```
    cpptask::event e;
    ...
    //somewhere in thread
    {
      e.wait();
      //do some work if event fired
    }
    ...
    //somewehere in another thread
    e.notify();
    ...
    ```

 * ***lockable_event*** class - Same as `cpptask::event` but should be used in pair with `process_lock` type to workload thread during waiting operation.

     Usage example:

     ```cpp
     ...
     cpptask::lockable_event e;
     ...
     //somewhere in thread
     {
       cpptask::process_lock<cpptask::lockable_event> lock(e); //will process other tasks during lock
       //do some work when event fired
     }
     ...
     //somewehere in another thread
     e.notify();
     ...
     ```
 * ***semaphore*** class - Counting semaphore synchronization abstraction. Object of this type can be used for example to limit threads count accessing some resource. Object can be initialized with initial counter value and maximum value for counter. When `lock` operation is called, counter value is decreased if counter value is non zero. When `unlock` operation is called, counter value is increased, but only in case if new value will be less or equal to maximum allowed. Also maximum value is negative by default, this mean that maximum value is unbounded.

     Usage example:

     ```
     ...
     cpptask::semaphore s(3,3); //We limit access to resource only to 3 threads concurrently
     SharedResource res;
     ...
     //somewhere in threads
     {
       std::unique_lock<cpptask::semaphore> lock(s);
       //now we can access resource
       process(res);
     }
     ```

 * ***lockable_semaphore*** class
 * ***process_lock*** class
3. **Algorithms**
 * ***range*** class
 * ***split_range*** function
 * ***split_num_range*** function
 * ***for_each*** function
 * ***reduce*** function
4. **Miscellaneous**
 * ***initializer*** class
 * ***exception*** class

# Supported Platforms (was tested on)

 * Linux (Arch)
  * gcc 5.1.0 (64bits)
  * clang 3.6.2 (64bits)
 * Windows
  * Visual Studio 2015 (64 bit)
  * MinGw-64 (gcc 5.1)

Also you can visit [c-vision.com.ua](http://c-vision.com.ua) to learn more news about project.
