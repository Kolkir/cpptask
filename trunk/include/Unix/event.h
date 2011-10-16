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

#include <pthread.h>

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
        count = 0;
    }
    ~Event()
    {
        pthread_cond_destroy(&pcond);
        pthread_mutex_destroy(&pmutex);
    }
    void Wait()
    {
        const unsigned long NANOSEC_PER_MILLISEC = 1000000;
        int rc = 0;
        timespec spec;
        timeb currSysTime;
        ftime(&currSysTime);

        spec.tv_sec = static_cast<long>(currSysTime.time);
        spec.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
        spec.tv_nsec += time * NANOSEC_PER_MILLISEC;

        pthread_mutex_lock(&m_mutex);
        long count = m_count;
        while(!m_signaled && m_count == count)
        {
            if(time != INFINITE)
            {
                rc = pthread_cond_timedwait(&m_cond, &m_mutex, &spec);
            }
            else
            {
                pthread_cond_wait(&m_cond, &m_mutex);
            }
            if(rc != 0)
            {
                break;
            }
        }
        pthread_mutex_unlock(&m_mutex);
    }
    bool Check()
    {
        return false;
    }
    void Signal()
    {
        pthread_mutex_lock(&m_mutex);
        m_signaled = true;
        ++m_count;
        pthread_cond_broadcast(&m_cond);
        pthread_mutex_unlock(&m_mutex);
    }
    void Reset()
    {
        m_signaled = false;
    }
    friend int WaitForTwo(Event& firstEvent, Event& secondEvent);
private:
    Event(const Event&);
    const Event& operator=(const Event&);
private:
   HANDLE hEvent;
};

inline int WaitForTwo(Event& firstEvent, Event& secondEvent)
{
    HANDLE events[2];
    events[0] = firstEvent.hEvent;
    events[1] = secondEvent.hEvent;
    DWORD rez = ::WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (rez == WAIT_OBJECT_0)
    {
        return 0;
    }
    else if (rez == WAIT_OBJECT_0 + 1)
    {
        return 1;
    }
    return -1;
}


}

#endif
