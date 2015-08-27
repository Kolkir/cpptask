/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2015, Kyrylo Kolodiazhnyi
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

#ifndef _EVENT_MANGER_H_
#define _EVENT_MANGER_H_

#include <queue>

#include <condition_variable>

namespace cpptask
{
    enum class EventId : int
    {
        NoneEvent = -1,
        ThreadStopEvent = 0,
        NewTaskEvent = 1,
        TaskFinishedEvent = 2,
        CustomEvent = 3
    };

    class EventManager
    {
    public:
        explicit EventManager()
            : count { 0 }
        {
        }

        EventManager(const EventManager&) = delete;
        EventManager& operator=(const EventManager&) = delete;

        void notify(EventId eventId)
        {
            std::lock_guard<std::mutex> lock { guard };
            ++count;
            eventQueue.push(eventId);
            cv.notify_all();
        }

        template <class Predicate>
        void wait(Predicate&& predicate)
        {
            bool done = false;
            while (!done)
            {
                std::unique_lock<std::mutex> lock{ guard };
                cv.wait(lock, [&]() {return count > 0; });

                if (predicate(eventQueue.front()))
                {
                    eventQueue.pop();
                    --count;
                    done = true;
                    break;
                }
                else
                {
                    cv.notify_all();
                }
            }

        }

    private:
        size_t count;
        std::queue<EventId> eventQueue;
        std::condition_variable cv;
        std::mutex guard;
    };
}
#endif

