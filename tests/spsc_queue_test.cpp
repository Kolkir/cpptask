#include <gtest/gtest.h>

#include <cpptask/cpptask.h>

#include <thread>
#include <random>
#include <chrono>
#include <mutex>
#include <iterator>
#include <condition_variable>

namespace
{
#if defined(_DEBUG) || defined(DEBUG)
    template<class T>
    std::string queueToString(const cpptask::SPSCQueue<T>& queue)
    {
        std::string qs;

        queue.Iterate([&qs](T val, bool isHead, bool isTail, bool isCacheStart, bool isCacheMid)
        {
            qs += "[";
            if (isHead)
            {
                qs += "H ";
            }
            if (isTail)
            {
                qs += "T ";
            }
            if (isCacheStart)
            {
                qs += "CS ";
            }
            if (isCacheMid)
            {
                qs += "CM ";
            }
            qs += ": " + std::to_string(val);
            qs += "] ";
        });

        return qs;
    }
#endif
}

TEST(SPSCTest, PushPop)
{
    cpptask::SPSCQueue<int> queue;

    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);

    int x = 0;

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(1, x);

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(2, x);

    queue.Enqueue(4);

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(3, x);

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(4, x);

    ASSERT_FALSE(queue.Dequeue(x));
}

#if defined(_DEBUG) || defined(DEBUG)
TEST(SPSCTest, Structure)
{
    cpptask::SPSCQueue<int> queue(-1);

    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);
    queue.Enqueue(4);

    ASSERT_EQ("[H CS CM : -1] [: 1] [: 2] [: 3] [T : 4] ", queueToString(queue));

    int x = 0;

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(1, x);

    ASSERT_EQ("[CS CM : -1] [H : 1] [: 2] [: 3] [T : 4] ", queueToString(queue));

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(2, x);

    ASSERT_EQ("[CS CM : -1] [: 1] [H : 2] [: 3] [T : 4] ", queueToString(queue));

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(3, x);

    ASSERT_EQ("[CS CM : -1] [: 1] [: 2] [H : 3] [T : 4] ", queueToString(queue));

    queue.Enqueue(5);

    ASSERT_EQ("[CS : 1] [: 2] [H CM : 3] [: 4] [T : 5] ", queueToString(queue));

    queue.Enqueue(6); //first part of AllocNode works

    ASSERT_EQ("[CS : 2] [H CM : 3] [: 4] [: 5] [T : 6] ", queueToString(queue));

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(4, x);

    ASSERT_EQ("[CS : 2] [CM : 3] [H : 4] [: 5] [T : 6] ", queueToString(queue));

    ASSERT_TRUE(queue.Dequeue(x));
    ASSERT_EQ(5, x);

    ASSERT_EQ("[CS : 2] [CM : 3] [: 4] [H : 5] [T : 6] ", queueToString(queue));

    queue.Enqueue(7); //first part of AllocNode works

    ASSERT_EQ("[CS CM : 3] [: 4] [H : 5] [: 6] [T : 7] ", queueToString(queue));
}
#endif

TEST(SPSCTest, Parallel)
{
    cpptask::SPSCQueue<int> queue;
    const int N = 1000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 20);

    std::mutex m;
    std::condition_variable cv;
    bool producerFinished = false;

    std::thread producerThread([&]()
    {
        for (int i = 0; i < N; ++i)
        {
            queue.Enqueue(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        }
        std::unique_lock<std::mutex> lk(m);
        producerFinished = true;
        cv.notify_one();
    });

    std::vector<int> x;
    x.reserve(N);
    auto inserter = std::back_inserter(x);

    std::thread consumerThread([&]()
    {
        bool done = false;
        while (!done)
        {
            int x = 0;
            if (queue.Dequeue(x))
            {
                *inserter = x;
                ++inserter;
            }
            else
            {
                std::unique_lock<std::mutex> lk(m);
                done = cv.wait_for(lk, std::chrono::milliseconds(5), [&] {return producerFinished; });
            }
        }
    });

    producerThread.join();
    consumerThread.join();

    ASSERT_EQ(N, x.size());

    for (int i = 0; i < N; ++i)
    {
        ASSERT_EQ(i, x[i]);
    }
}
