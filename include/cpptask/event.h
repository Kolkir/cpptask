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

#include "mutex.h"
#include "waitoneof.h"

#include <condition_variable>

namespace cpptask
{

class event : public MultWaitBase<event>
{
public:
    using native_handle_type = std::condition_variable::native_handle_type;

    event() noexcept
        : signaled(false)
    {}
    
    ~event() {}

    event(const event&) = delete;
    const event& operator=(const event&) = delete;

    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        cv.wait(lock, [&] {return signaled; });
    }

    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& duration)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return cv.wait_for(lock, duration, [&] {return signaled; });
    }

    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& time)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        cv.wait_until(lock, time, duration, [&] {return signaled; });
    }

    void notify()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            signaled = true;
        }
        cv.notify_all();

        MultSignal();
    }
    void reset()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        signaled = false;
    }

    native_handle_type native_handle()
    {
        return cv.native_handle();
    }

protected:
    virtual bool MultCheck()
    {
        return wait_for(std::chrono::milliseconds(0));
    }

private:
    std::condition_variable cv;
    std::mutex _mutex;
    bool signaled;
};

}

#endif
