**Summary**

"cpptask" is an open source library which represents a higher-level, task-based parallelism that abstracts platform details and threading mechanisms. 

It was made to be similar to native C++ "async" interfaces as much as possible. The main advantages are:

1. You can specify number of threads, library will not create new ones.
2. "Work stealing" technique is used for load balancing.
3. You can use special type of synchronization primitives and special lock type to guarantee workload of thread during lock waiting operation.

Library provide two main features:
 * "future" class
 * "async" function
These features have same signatures as standard C++ analogs. 

**Implementation**

It is implemented as headers only library for simple integration to projects.

**Supported Platforms (was tested on)**

 * Linux (Arch)
  * gcc 5.1.0 (64bits)
 * Windows
  * Visual Studio 2015 (64 bit)
  * MinGw-64 (gcc 5.1)

**Features**

 * Event primitive
 * Semaphore primitive
 * Work-Stealing tasks management 
 * Parallel-For algorithm
 * Parallel-Reduce algorithm
 
You can learn more on the [UserGuide](https://github.com/Kolkir/cpptask/wiki/UserGuide) page.
Also you can visit [c-vision.com.ua](http://c-vision.com.ua) to learn more news about project.
