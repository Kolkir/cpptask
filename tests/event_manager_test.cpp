#include <gtest/gtest.h>

#include <cpptask/eventmanager.h>

#include <future>
#include <vector>
#include <iostream>

namespace 
{
    std::mutex ioGuard;

    template<typename T>
    void writeLog(T&& val)
    {
        std::cout << std::forward<T>(val);
    }

    template<typename T, typename... Ts>
    void writeLog(T&& val, Ts&&... vals)
    {
        writeLog(std::forward<T>(val));
        writeLog(std::forward<Ts>(vals)...);
    }

    template<typename T, typename... Ts>
    void log(T&& val, Ts&&... vals)
    {
        std::lock_guard<std::mutex> lock(ioGuard);
        writeLog(std::forward<T>(val), std::forward<Ts>(vals)...);
        std::cout.flush();
    }
}

TEST(EventManagerTest, SendWait)
{
    cpptask::EventManager mngr;

    const int N = 10;

    for (int i = 0; i < N; ++i)
    {
        std::vector<std::future<void>> waits;
        log("session start\n");
        for (int j = 0; j < 2; ++j)
        {
            waits.emplace_back(std::async(std::launch::async, [&mngr]
            {
                mngr.notify(cpptask::EventId::NewTaskEvent);
                log("sent NewTask, waiting ThreadStop ", std::this_thread::get_id(), "\n");
                mngr.wait([](cpptask::EventId eventId)
                {
                    return eventId == cpptask::EventId::ThreadStopEvent;
                });
                log("ThreadStop received ", std::this_thread::get_id(), "\n");
            }));
        }
        for (int j = 0; j < 2; ++j)
        {
            waits.emplace_back(std::async(std::launch::async, [&mngr]
            {
                log("waiting NewTask ", std::this_thread::get_id(), "\n");
                mngr.wait([](cpptask::EventId eventId)
                {
                    return eventId == cpptask::EventId::NewTaskEvent;
                });
                mngr.notify(cpptask::EventId::ThreadStopEvent);
                log("sent ThreadStop ", std::this_thread::get_id(), "\n");
            }));
        }

        for (auto& f : waits)
        {
            f.get();
        }

        log("\n");
    }
}

TEST(EventManagerTest, SendWaitBroadcast)
{
    cpptask::EventManager mngr;

    const int N = 10;

    for (int i = 0; i < N; ++i)
    {
        std::vector<std::future<void>> waits;
        log("session start\n");
        const int nt = 3;
        waits.emplace_back(std::async(std::launch::async, [&nt,&mngr]
        {
            for (int t = 0; t < nt; ++t)
            {
                mngr.notify(cpptask::EventId::NewTaskEvent);
                log("sent ", static_cast<int>(cpptask::EventId::NewTaskEvent), " ", std::this_thread::get_id(), "\n");
            }
            
            for (int t = 0; t < nt; ++t)
            {
                mngr.wait([](cpptask::EventId eventId)
                {
                    return eventId == cpptask::EventId::TaskFinishedEvent;
                });
                log(static_cast<int>(cpptask::EventId::TaskFinishedEvent), " received ", std::this_thread::get_id(), "\n");
            }

            for (int t = 0; t < nt; ++t)
            {
                mngr.notify(cpptask::EventId::ThreadStopEvent);
            }
        }));


        for (int t = 0; t < nt; ++t)
        {
            waits.emplace_back(std::async(std::launch::async, [&mngr]
            {
                bool done = false;
                while (!done)
                {
                    cpptask::EventId id = cpptask::EventId::NoneEvent;
                    mngr.wait([&id](cpptask::EventId eventId)
                    {
                        auto res = eventId == cpptask::EventId::NewTaskEvent ||
                                   eventId == cpptask::EventId::ThreadStopEvent;
                        id = eventId;
                        return res;
                    });
                    log(static_cast<int>(id), " received ", std::this_thread::get_id(), "\n");
                    if (id == cpptask::EventId::ThreadStopEvent)
                    {
                        done = true;
                    }
                    else
                    {
                        mngr.notify(cpptask::EventId::TaskFinishedEvent);
                        log("sent ", static_cast<int>(cpptask::EventId::TaskFinishedEvent), " ", std::this_thread::get_id(), "\n");
                    }
                }
            }));
        }

        for (auto& f : waits)
        {
            f.get();
        }

        log("\n");
    }
}
