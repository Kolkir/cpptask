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

#ifndef _WAITONEBASE_H_
#define _WAITONEBASE_H_

#include <vector>
#include <mutex>
#include <algorithm>

namespace cpptask
{
    template <class E>
    class WaitOneBase
    {
    public:

        WaitOneBase() {}

        virtual ~WaitOneBase() {}

        WaitOneBase(const WaitOneBase&) = delete;

        WaitOneBase& operator=(const WaitOneBase&) = delete;

        template <class Container>
        friend int wait_one_of(Container container);

        template <class SyncIterator>
        friend int wait_one_of(SyncIterator start, SyncIterator end);

    protected:

        void AddWaitEvent(E& event)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            waitEvents.push_back(&event);
        }

        void DelWaitEvent(E* event)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            typename std::vector<E*>::iterator i = std::find(this->waitEvents.begin(),
                this->waitEvents.end(),
                event);
            if (i != this->waitEvents.end())
            {
                this->waitEvents.erase(i);
            }
        }

        void MultSignal()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            typename std::vector<E*>::iterator i = waitEvents.begin(),
                e = waitEvents.end();
            for (; i != e; ++i)
            {
                (*i)->notify();
            }
        }

        virtual bool MultCheck() = 0;

    private:
        std::mutex _mutex;
        std::vector<E*> waitEvents;
    };
}

#endif
#pragma once
