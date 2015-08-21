#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>
#include <mutex>
#include <iostream>

namespace
{
    std::mutex guard;

    void processFunc(CppTaskTestData::ArrayType& arr)
    {
        auto i = arr.begin();
        auto e = arr.end();
        for (; i != e; ++i)
        {
            CppTaskTestData::DoubleSqrt()(*i);
        }
    }
    void processFuncLock()
    {
        std::lock_guard<std::mutex> lock(guard);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void processFuncLock2()
    {
        cpptask::process_lock<std::mutex> lock(guard); //allow use thread for calculations
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST_F(CppTaskTest, ProcessLock_Serial)
{
    cpptask::TaskThreadPool threadPool(1);

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
    cpptask::TaskThreadPool threadPool(1);

    std::unique_lock<std::mutex> lock(guard);
    auto f1 = cpptask::async(std::launch::async, processFuncLock2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // additional thread blocked, he stole task
    ASSERT_NO_THROW(cpptask::for_each(testArray2.begin(), testArray2.end(), CppTaskTestData::DoubleSqrt(), 4)); //generate more tasks
    lock.unlock(); //unlock after second tasks done

    f1.wait();

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}