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

#include "../mutex.h"

#include <sys/timeb.h>
#include <pthread.h>
#include <cerrno>

#include <vector>
#include <algorithm>
#include <time.h>

namespace cpptask
{

class Event
{
public:
    Event()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutex_init(&pmutex, &attr);
        pthread_mutexattr_destroy(&attr);
        pthread_cond_init(&pcond, NULL);
        signaled = false;
    }
    ~Event()
    {
        pthread_cond_destroy(&pcond);
        pthread_mutex_destroy(&pmutex);
    }
    void Wait()
    {
        int rc = 0;
        pthread_mutex_lock(&pmutex);
        while (!signaled)
        {
            rc = pthread_cond_wait(&pcond, &pmutex);
            if (rc != 0)
            {
                break;
            }
        }
        pthread_mutex_unlock(&pmutex);
    }
    bool Check()
    {
        bool isSignaled = false;
        pthread_mutex_lock(&pmutex);
        isSignaled = signaled;
        pthread_mutex_unlock(&pmutex);
        return isSignaled;
    }
    void Signal()
    {
        pthread_mutex_lock(&pmutex);
        if (!signaled)
        {
            signaled = true;
            pthread_cond_broadcast(&pcond);

            std::vector<Event*>::iterator i = waitEvents.begin(),
                                          e = waitEvents.end();
            for (;i != e; ++i)
            {
                (*i)->Signal();
            }
        }
        pthread_mutex_unlock(&pmutex);
    }
    void Reset()
    {
        pthread_mutex_lock(&pmutex);
        signaled = false;
        pthread_mutex_unlock(&pmutex);
    }
    friend inline int WaitForMultiple(std::vector<Event*>& events);
private:
    Event(const Event&);
    const Event& operator=(const Event&);

    void AddWaitEvent(Event* event)
    {
        pthread_mutex_lock(&pmutex);
        waitEvents.push_back(event);
        pthread_mutex_unlock(&pmutex);
    }
    void DelWaitEvent(Event* event)
    {
        pthread_mutex_lock(&pmutex);
        std::vector<Event*>::iterator i = std::find(waitEvents.begin(),
                                                    waitEvents.end(),
                                                    event);
        if (i != waitEvents.end())
        {
            waitEvents.erase(i);
        }
        pthread_mutex_unlock(&pmutex);
    }
private:
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
    bool signaled;
    //multiple wait implementation
    std::vector<Event*> waitEvents;
};

inline int WaitForMultiple(std::vector<Event*>& events)
{
    Event commonEvent;

    std::vector<Event*>::iterator i = events.begin();
    std::vector<Event*>::iterator e = events.end();

    int index = -1;
    for (; i != e; ++i, ++index)
    {
        (*i)->AddWaitEvent(&commonEvent);
        if ((*i)->Check())
        {
            ++index;
            break;
        }
    }

    if (index == -1)
    {
        commonEvent.Wait();
    }

    i = events.begin();
    if (index == -1)
    {
        for (; i != e; ++i, ++index)
        {
            if ((*i)->Check())
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
