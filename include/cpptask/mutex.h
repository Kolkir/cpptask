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

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "taskmanager.h"

#include <mutex>

namespace cpptask
{

//use this class only if you have corresponding process_lock object
class mutex
{
public:
    typedef EventManager EventManagerType;

    mutex() noexcept
    {}

    ~mutex()
    {}

    mutex(const mutex&) = delete;

    const mutex& operator=(const mutex&) = delete;

    void lock()
    {
       guard.lock();
    }

    void unlock()
    {
        guard.unlock();
        TaskManager::GetCurrent().GetEventManager().notify(EventId::CustomEvent);
    }

    bool try_lock()
    {
        return guard.try_lock();
    }

private:
    std::mutex guard;
};

}

#endif
