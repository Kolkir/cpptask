**Summary**

"cpptask" is an open source library which represents a higher-level, task-based parallelism that abstracts platform details and threading mechanisms. It is similar to Intel TBB or Microsoft Parallel Pattern Library but much more lightweight and less powerful.

**Implementation**

It is implemented as headers only library for simple integration to projects.

**Supported Platforms (was tested on)**

 * Windows
  * MSVS2008 (32bits and 64bits)
  * MSVS2010 (32bits and 64bits)
  * MSVS2010 Express (32bits)
  * mingw(gcc-4.5.2) (32bits and 64bits)
 * Linux (Debian)
  * gcc 4.6.1 (32bits)

**Features**

 * Thread class
 * Atomic variables
 * Mutex primitive
 * Event primitive
 * Semaphore primitive
 * Task class for faster delivering to threads
 * Work-Stealing tasks management 
 * Parallel-For algorithm
 * Parallel-Reduce algorithm
 * Parallel-Invoke algorithm
 * Delivering exceptions from threads and tasks to caller
 * Support of lambda expressions

You can learn more on the [UserGuide](https://github.com/Kolkir/cpptask/wiki/UserGuide) page.
Also you can visit [c-vision.com.ua](http://c-vision.com.ua) to learn more news about project.
