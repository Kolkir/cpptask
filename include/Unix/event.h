/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2011, Kirill Kolodyazhnyi
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
#include "multwait.h"
#include <pthread.h>

namespace cpptask
{

class Event : public MultWaitBase<Event, Mutex>
{
public:
    Event()
    {
        pthread_cond_init(&pcond, NULL);
        signaled = false;
    }
    ~Event()
    {
        pthread_cond_destroy(&pcond);
    }
    void Wait()
    {
        int rc = 0;
        Lock();
        while (!signaled)
        {
            rc = pthread_cond_wait(&pcond, GetMutex());
            if (rc != 0)
            {
                break;
            }
        }
        UnLock();
    }
    bool Check()
    {
        bool isSignaled = false;
        Lock();
        isSignaled = signaled;
        UnLock();
        return isSignaled;
    }
    void Signal()
    {
        Lock();
        if (!signaled)
        {
            signaled = true;
            pthread_cond_broadcast(&pcond);

            MultSignal();
        }
        UnLock();
    }
    void Reset()
    {
        Lock();
        signaled = false;
        UnLock();
    }

    virtual bool MultCheck()
    {
        return Check();
    }

private:
    Event(const Event&);
    const Event& operator=(const Event&);

private:
    pthread_cond_t pcond;
    bool signaled;
};

}

#endif
