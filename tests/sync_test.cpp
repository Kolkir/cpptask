#include <gtest/gtest.h>

#include <cpptask/mutex.h>
#include <cpptask/event.h>
#include <cpptask/semaphor.h>
#include <cpptask/waitoneof.h>

#include <future>


TEST(SyncTest, Mutex)
{
    cpptask::mutex m;
    {
        std::lock_guard<cpptask::mutex> lock(m);
        auto f = std::async([&]()
        {
            ASSERT_FALSE(m.try_lock());
        });
        f.wait();
    }
    ASSERT_TRUE(m.try_lock());
    m.unlock();
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

TEST(SyncTest, MutexMultipleImmediateUnlock)
{
    cpptask::mutex m1;
    cpptask::mutex m2;
    std::lock_guard<cpptask::mutex> lock(m1);

    m2.lock();
    auto f = std::async([&]()
    {
        cpptask::wait_array mutexes = {&m1, &m2};
        int res = cpptask::wait_one_of(mutexes);
        ASSERT_EQ(1, res);
    });
    m2.unlock();

    f.wait();
}

TEST(SyncTest, MutexMultipleWaitUnlock)
{
    cpptask::mutex m1;
    cpptask::mutex m2;
    std::lock_guard<cpptask::mutex> lock(m2);

    m1.lock();
    auto f = std::async([&]()
    {
        cpptask::wait_array mutexes = {&m1, &m2};
        int res = cpptask::wait_one_of(mutexes);
        ASSERT_EQ(0, res);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    m1.unlock();

    f.wait();
}

TEST(SyncTest, Event)
{
    cpptask::event m;
    {
        m.notify();
        auto f = std::async([&]()
        {
            ASSERT_TRUE(m.wait_for(std::chrono::milliseconds(0)));
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

    e2.notify();
    auto f = std::async([&]()
    {
        cpptask::wait_array events = {&e1, &e2};
        int res = cpptask::wait_one_of(events);
        ASSERT_EQ(1, res);
    });
    f.wait();

    e1.notify();
    e2.notify();
}

TEST(SyncTest, EventMultipleWaitUnlock)
{
    cpptask::event e1;
    cpptask::event e2;

    auto f = std::async([&]()
    {
        cpptask::wait_array events = {&e1, &e2};
        int res = cpptask::wait_one_of(events.begin(), events.end());
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
            ASSERT_TRUE(s.wait_for(std::chrono::milliseconds(0)));
        });

        auto f2 = std::async([&]()
        {
            ASSERT_TRUE(s.wait_for(std::chrono::milliseconds(0)));
        });

        s.notify();
        auto f3 = std::async([&]()
        {
            ASSERT_TRUE(s.wait_for(std::chrono::milliseconds(0)));
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

    auto f = std::async([&]()
    {
        cpptask::wait_array events = { &e, &s };
        int res = cpptask::wait_one_of(events);
        ASSERT_EQ(1, res);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    s.notify();
    s.notify();
    s.notify();

    f.wait();

    e.notify();
}
