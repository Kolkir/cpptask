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

#ifndef _EVENT_H_
#define _EVENT_H_

#include "taskmanager.h"

#include <condition_variable>

namespace cpptask
{

class event
{
public:

    event() noexcept
        : signaled(false)
    {}

    ~event()
    {}

    event(const event&) = delete;
    const event& operator=(const event&) = delete;

    void notify()
    {
        std::lock_guard<std::mutex> lock(guard);
        signaled = true;
        cv.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(guard);
        cv.wait(lock, [&] {return signaled; });
        signaled = false;        
    }

    bool check()
    {
        std::unique_lock<std::mutex> lock(guard);
        if (cv.wait_for(lock, std::chrono::milliseconds(0), [&] {return signaled; }))
        {
            signaled = false;
            cv.notify_all();
            return true;
        }
        return false;
    }

private:
    std::condition_variable cv;
    std::mutex guard;
    bool signaled;
};

//use this class only if you have corresponding process_lock object
class lockable_event
{
public:
    typedef EventManager EventManagerType;

    lockable_event() noexcept
    {}
    
    ~lockable_event()
    {}

    lockable_event(const lockable_event&) = delete;
    const lockable_event& operator=(const lockable_event&) = delete;

    void notify()
    {
        evt.notify();
        TaskManager::GetCurrent().GetEventManager().notify(EventId::CustomEvent);
    }

    void lock()
    {
        evt.wait();
    }

    void unlock()
    {
        //do nothing
    }

    bool try_lock()
    {
        return evt.check();
    }

private:
    event evt;
};

}

#endif
