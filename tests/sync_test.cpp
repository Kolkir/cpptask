#include <gtest/gtest.h>
/*
#include <cpptask/event.h>
#include <cpptask/semaphor.h>
#include <cpptask/waitoneof.h>

#include <future>

TEST(SyncTest, Event)
{
    cpptask::event m;
    {
        m.notify();
        auto f = std::async([&]()
        {
            ASSERT_TRUE(m.check());
        });
        f.wait();
    }
    m.notify();
    m.notify();
}

TEST(SyncTest, EventMultipleImmediateUnlock)
{
    cpptask::event e1;
    cpptask::event e2;

    cpptask::wait_one_of waits{&e1, &e2};

    e2.notify();
    auto f = std::async([&]()
    {        
        ASSERT_EQ(1, waits.wait());
    });
    f.wait();

    e1.notify();
    e2.notify();
}

TEST(SyncTest, EventMultipleWaitUnlock)
{
    cpptask::event e1;
    cpptask::event e2;

    cpptask::wait_one_of waits {&e1, &e2 };

    auto f = std::async([&]()
    {        
        int res = waits.wait();
        ASSERT_EQ(0, res);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    e1.notify();

    f.wait();
}


TEST(SyncTest, Semaphore)
{
    cpptask::semaphore s(2);
    {

        auto f1 = std::async([&]()
        {
            ASSERT_TRUE(s.check());
        });

        auto f2 = std::async([&]()
        {
            ASSERT_TRUE(s.check());
        });

        s.notify();
        auto f3 = std::async([&]()
        {
            ASSERT_TRUE(s.check());
        });

        f1.wait();
        f2.wait();
        f3.wait();
    }
}

TEST(SyncTest, SemaphoreMultipleWaitUnlock)
{
    cpptask::event e;
    cpptask::semaphore s;

    cpptask::wait_one_of waits{&e,&s};

    auto f = std::async([&]()
    {        
        int res = waits.wait();
        ASSERT_EQ(1, res);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    s.notify();
    s.notify();
    s.notify();

    f.wait();

    e.notify();
}
*/
