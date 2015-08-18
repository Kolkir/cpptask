/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2011, Kyrylo Kolodiazhnyi
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _MULTWAIT_H_
#define _MULTWAIT_H_

#include <vector>
#include <mutex>
#include <condition_variable>

namespace cpptask
{

    class wait_one_of;

    typedef WaitOneBase<wait_one_of> wait_event;
    typedef std::vector<wait_event*> wait_array;

    class wait_one_of
    {
    public:
        wait_one_of()
            :lastEvent(nullptr)
        {}

        wait_one_of(std::initializer_list<wait_event*> inputs)
            : events(inputs)
            , lastEvent(nullptr)
        {
            std::unique_lock<std::mutex> lock(guard);
            for (auto e : events)
            {
                e->addWaiter(*this);
            }
        }

        ~wait_one_of()
        {
            for(auto e : events)
            {
                e->removeWaiter(*this);
            }
        }

        wait_one_of(const wait_one_of&) = delete;
        wait_one_of& operator=(const wait_one_of&) = delete;

        void addEvent(wait_event& we)
        {
            std::unique_lock<std::mutex> lock(guard);
            events.push_back(&we);
            we.addWaiter(*this);
        }

        bool notify(wait_event& e)
        {
           std::unique_lock<std::mutex> lock(guard, std::try_to_lock);
           if (lock)
           {
               lastEvent = &e;
               cv.notify_one();
               return true;
           }
           return false;
        }

        int wait()
        {
            std::unique_lock<std::mutex> lock(guard);
            cv.wait(lock,[&](){return lastEvent != nullptr;});
            lastEvent->waitBase();//required for semaphore to decrease counter
            auto i = std::find(events.begin(), events.end(), lastEvent);
            auto index = static_cast<int>(std::distance(events.begin(), i));
            lastEvent = nullptr;
            return index;
        }
    private:
        std::mutex guard;
        std::condition_variable cv;
        wait_array events;
        wait_event* lastEvent;
    };
}

#endif
