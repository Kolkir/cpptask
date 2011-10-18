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
        Wait(0, true);
    }
    bool Check()
    {
        return Wait(0, false);
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

    bool Wait(unsigned long time, bool infiniteWait)
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
            if(!infiniteWait)
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
        if (rc == ETIMEDOUT && time != INFINITE)
        {
            return false;
        }
        return true;
    }
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    bool m_signaled;
    int m_count;
};

inline int WaitForTwo(Event& firstEvent, Event& secondEvent)
{
    //create common Event
    //firstEvent - add common Event
    //secondEvent - add common Event
    //wait for common Event

    //when signal event - signal all common events in list
    //add list of events to event
    return -1;
}


}

#endif
