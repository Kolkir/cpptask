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

#ifndef _MUTEX_SELECT_H_
#define _MUTEX_SELECT_H_

#include "waitoneof.h"
#include "event.h"

#include <mutex>

namespace cpptask
{
    class mutex : public MultWaitBase<event>
    {
    public:
        using native_handle_type = std::mutex::native_handle_type;

        mutex() noexcept {}
        ~mutex() {}

        mutex(const mutex&) = delete;
        mutex& operator=(const mutex&) = delete;

        void lock()
        {
            _mutex.lock();
        }

        bool try_lock()
        {
            return _mutex.try_lock();
        }

        void unlock()
        {
            _mutex.unlock();
            MultSignal();
        }
        
        native_handle_type native_handle()
        {
            return _mutex.native_handle();
        }

        virtual bool MultCheck()
        {
            auto res = try_lock();
            if (res)
            {
                unlock();
            }
            return res;
        }

    private:
        std::mutex _mutex;
    };
}
#endif
