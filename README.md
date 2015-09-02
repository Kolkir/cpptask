**Summary**

"cpptask" is an open source library which represents a higher-level, task-based parallelism that abstracts platform details and threading mechanisms.

It was made to be similar to native C++ "async" interfaces as much as possible. The main advantages are:

1. You can specify number of threads, library will not create new ones.
2. "Work stealing" technique is used for load balancing.
3. You can use special type of synchronization primitives and special lock type to guarantee workload of thread during lock waiting operation.
4. Library implemented as headers only library.

**Library provide next features:**

1. Task management
 * ***future*** class
 * ***async*** function 
 
 These functionality copies standard C++ signatures, methods and behaviour except `shared_future` class. Usage example:
 
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
 
2. Synchronization
 * **_mutex_** class
 * **_event_** class
 * ***lockable_event*** class
 * **_semaphore_** class
 * ***lockable_semaphore*** class
 * ***process_lock*** class
3. Algorithms
 * **_range_** class
 * ***split_range*** function
 * ***split_num_range*** function
 * ***for_each*** function
 * **_reduce_** function
4. Miscellaneous
 * **_initializer_** class
 * **_exception_** class

**Supported Platforms (was tested on)**

 * Linux (Arch)
  * gcc 5.1.0 (64bits)
 * Windows
  * Visual Studio 2015 (64 bit)
  * MinGw-64 (gcc 5.1)

Also you can visit [c-vision.com.ua](http://c-vision.com.ua) to learn more news about project.
