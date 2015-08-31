#include <gtest/gtest.h>

#include <cpptask/cpptask.h>

#include <future>

TEST(SyncTest, Event)
{
    int count = 0;
    cpptask::event e1;
    cpptask::event e2;
    {
        auto f = std::async(std::launch::async, [&]()
        {
            while (count != 5)
            {
                e1.wait();
                ++count;            
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                e2.notify();
            }
        });

        for (int i = 0; i < 5; ++i)
        {
            e1.notify();
            e2.wait();
        }
        f.wait();
    }
    ASSERT_EQ(5, count);
}

TEST(SyncTest, Semaphore)
{
    cpptask::semaphore s(2,2);
    {
        auto f1 = std::async(std::launch::async,[&]()
        {
            s.lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            s.unlock();
        });

        auto f2 = std::async(std::launch::async,[&]()
        {
            s.lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            s.unlock();
        });

        auto f3 = std::async(std::launch::async,[&]()
        {
            s.lock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            s.unlock();
        });

        f1.wait();
        f2.wait();
        f3.wait();
    }
}
