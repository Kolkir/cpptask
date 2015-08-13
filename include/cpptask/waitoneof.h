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

#include "event.h"

#include <vector>

namespace cpptask
{

    typedef std::vector<WaitOneBase<event>*> wait_array;

    template <class Container>
    int wait_one_of(Container container)
    {
        return wait_one_of(std::begin(container), std::end(container));
    }

    template <class SyncIterator>
    int wait_one_of(SyncIterator start, SyncIterator end)
    {
        event commonEvent;

        auto i = start;
        auto e = end;

        int index = -1;
        bool needWait = true;
        for (int j = 0; i != e; ++i, ++j)
        {
            assert((*i) != nullptr);
            if ((*i) != nullptr)
            {
                (*i)->AddWaitEvent(commonEvent);
                if ((*i)->MultCheck())
                {
                    index = j;
                    needWait = false;
                    break;
                }
            }
        }

        if (needWait)
        {
            commonEvent.wait();
        }

        i = start;
        if (index == -1)
        {
            for (int j = 0; i != e; ++i, ++j)
            {
                assert((*i) != nullptr);
                if ((*i) != nullptr)
                {
                    if ((*i)->MultCheck())
                    {
                        index = j;
                        break;
                    }
                }
            }
        }

        i = start;
        for (; i != e; ++i)
        {
            assert((*i) != nullptr);
            if ((*i) != nullptr)
            {
                (*i)->DelWaitEvent(&commonEvent);
            }
        }

        return index;
    }
}

#endif