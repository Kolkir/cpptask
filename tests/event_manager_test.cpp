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

    const int N = 100;

    for (int i = 0; i < N; ++i)
    {
        std::vector<std::future<void>> waits;
        log("session start\n");
        waits.emplace_back(std::async(std::launch::async, [&mngr]
        {
            mngr.notify(100);
            log("sent 100, waiting 101\n");
            mngr.wait([](int eventId)
            {
                return eventId == 101;
            });
            log("101 received\n");
        }));

        waits.emplace_back(std::async(std::launch::async, [&mngr]
        {
            mngr.notify(200);
            log("sent 200, waiting 201\n");
            mngr.wait([](int eventId)
            {
                return eventId == 201;
            });
            log("201 received\n");
        }));

        waits.emplace_back(std::async(std::launch::async, [&mngr]
        {
            log("waiting 100\n");
            mngr.wait([](int eventId)
            {
                return eventId == 100;
            });
            mngr.notify(101);
            log("sent 101\n");
        }));

        waits.emplace_back(std::async(std::launch::async, [&mngr]
        {
            log("waiting 200\n");
            mngr.wait([](int eventId)
            {
                return eventId == 200;
            });
            mngr.notify(201);
            log("sent 201\n");
        }));

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
                mngr.notify(100 + t);
                log("sent ", 100 + t, " ", std::this_thread::get_id(), "\n");
            }
            
            for (int t = 0; t < nt; ++t)
            {
                int id = 0;
                mngr.wait([&id](int eventId)
                {
                    auto res =  eventId >= 200;
                    id = eventId;
                    return res;
                });
                log(id , " received ", std::this_thread::get_id(), "\n");
            }

            for (int t = 0; t < nt; ++t)
            {
                mngr.notify(300);
            }
        }));


        for (int t = 0; t < nt; ++t)
        {
            waits.emplace_back(std::async(std::launch::async, [&mngr]
            {
                bool done = false;
                while (!done)
                {
                    int id = 0;
                    mngr.wait([&id](int eventId)
                    {
                        auto res = (eventId >= 100 && eventId < 200) || eventId == 300;
                        id = eventId;
                        return res;
                    });
                    log(id, " received ", std::this_thread::get_id(), "\n");
                    if (id == 300)
                    {
                        done = true;
                    }
                    else
                    {
                        mngr.notify(200 + (id - 100));
                        log("sent ", 200 + (id - 100), " ", std::this_thread::get_id(), "\n");
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