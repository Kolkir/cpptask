#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>
#include <mutex>
#include <iostream>

namespace
{
    std::mutex guard;
    cpptask::mutex guard2;
    cpptask::lockable_event event1;
    cpptask::lockable_semaphore sem1(1,2);

    void processFuncLock()
    {
        std::lock_guard<std::mutex> lock(guard);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void processFuncLockMutex()
    {
        cpptask::process_lock<cpptask::mutex> lock(guard2); //allow use thread for calculations
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void processFuncLockEvent()
    {
        cpptask::process_lock<cpptask::lockable_event> lock(event1); //allow use thread for calculations
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void processFuncLockSem()
    {
        cpptask::process_lock<cpptask::lockable_semaphore> lock(sem1); //allow use thread for calculations
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST_F(CppTaskTest, ProcessLock_Serial)
{
    cpptask::initializer init(1);

    std::unique_lock<std::mutex> lock(guard);
    auto f1 = cpptask::async(std::launch::async, processFuncLock);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // additional thread blocked, he stole task
    ASSERT_NO_THROW(cpptask::for_each(testArray2.begin(), testArray2.end(), CppTaskTestData::DoubleSqrt(), 4)); //generate more tasks
    lock.unlock(); //unlock after second tasks done
    
    f1.wait();

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}

TEST_F(CppTaskTest, ProcessLock_Parallel)
{
    cpptask::initializer init(1);

    std::unique_lock<cpptask::mutex> lock(guard2);
    auto f1 = cpptask::async(std::launch::async, processFuncLockMutex);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // additional thread blocked, he stole task
    ASSERT_NO_THROW(cpptask::for_each(testArray2.begin(), testArray2.end(), CppTaskTestData::DoubleSqrt(), 4)); //generate more tasks
    lock.unlock(); //unlock after second tasks done

    f1.wait();

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}

TEST_F(CppTaskTest, ProcessLock_ParallelEvent)
{
    cpptask::initializer init(1);

    auto f1 = cpptask::async(std::launch::async, processFuncLockEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // additional thread blocked, he stole task
    ASSERT_NO_THROW(cpptask::for_each(testArray2.begin(), testArray2.end(), CppTaskTestData::DoubleSqrt(), 4)); //generate more tasks
    event1.notify(); //unlock after second tasks done

    f1.wait();

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}

TEST_F(CppTaskTest, ProcessLock_ParallelSemaphore)
{
    cpptask::initializer init(1);

    std::unique_lock<cpptask::lockable_semaphore> lock(sem1);
    auto f1 = cpptask::async(std::launch::async, processFuncLockSem);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // additional thread blocked, he stole task
    ASSERT_NO_THROW(cpptask::for_each(testArray2.begin(), testArray2.end(), CppTaskTestData::DoubleSqrt(), 4)); //generate more tasks
    lock.unlock(); //unlock after second tasks done

    f1.wait();

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}
