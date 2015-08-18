/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2012, Kyrylo Kolodiazhnyi
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

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "waitonebase.h"
#include "waitoneof.h"

#include <condition_variable>

namespace cpptask
{
    class semaphore : public WaitOneBase<wait_one_of>
    {
    public:
        explicit semaphore(size_t n = 0)
            : count{ n }
        {

        }

        semaphore(const semaphore&) = delete;
        semaphore& operator=(const semaphore&) = delete;

        void notify()
        {
            std::lock_guard<std::mutex> lock{ guard };
            ++count;
            cv.notify_one();
            notifyWaiters();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock{ guard };
            cv.wait(lock, [&] { return count > 0; });
            --count;
        }

        bool check()
        {
            std::unique_lock<std::mutex> lock(guard);
            return count > 0;
        }

        virtual void waitBase()
        {
            wait();
        }

    protected:

        virtual std::mutex& getMutex()
        {
            return guard;
        }

    private:
        size_t count;
        std::condition_variable cv;
        std::mutex guard;
    };
}
#endif
