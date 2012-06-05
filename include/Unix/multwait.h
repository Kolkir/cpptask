/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2012, Kirill Kolodyazhnyi
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
#include <algorithm>

namespace cpptask
{

template <class E, class M>
class MultWaitBase
{
public:
    MultWaitBase()
    {
    }

    virtual ~MultWaitBase()
    {
    }

    void Lock()
    {
        this->mutex.Lock();
    }

    void UnLock()
    {
        this->mutex.UnLock();
    }

    void AddWaitEvent(E* event)
    {
        Lock();
        waitEvents.push_back(event);
        UnLock();
    }

    void DelWaitEvent(E* event)
    {
        Lock();
        typename std::vector<E*>::iterator i = std::find(this->waitEvents.begin(),
                                                    this->waitEvents.end(),
                                                    event);
        if (i != this->waitEvents.end())
        {
            this->waitEvents.erase(i);
        }
        UnLock();
    }

    void MultSignal()
    {
        typename std::vector<E*>::iterator i = waitEvents.begin(),
                                      e = waitEvents.end();
        for (;i != e; ++i)
        {
            (*i)->Signal();
        }
    }

    virtual bool MultCheck() = 0;

    pthread_mutex_t* GetMutex()
    {
        return mutex.GetNative();
    }
private:
    M mutex;
    std::vector<E*> waitEvents;
};

template <class E, class M>
int WaitForMultiple(std::vector<MultWaitBase<E,M>*>& events)
{
    E commonEvent;

    typename std::vector<MultWaitBase<E, M>*>::iterator i = events.begin();
    typename std::vector<MultWaitBase<E,M>*>::iterator e = events.end();

    int index = -1;
    bool needWait = true;
    for (; i != e; ++i, ++index)
    {
        (*i)->AddWaitEvent(&commonEvent);
        if ((*i)->MultCheck())
        {
            ++index;
            needWait = false;
            break;
        }
    }

    if (needWait)
    {
        commonEvent.Wait();
    }

    i = events.begin();
    if (index == -1)
    {
        for (; i != e; ++i, ++index)
        {
            if ((*i)->MultCheck())
            {
                ++index;
                break;
            }
        }
    }

    for (; i != e; ++i, ++index)
    {
        (*i)->DelWaitEvent(&commonEvent);
    }

    return index;
}


}

#endif
