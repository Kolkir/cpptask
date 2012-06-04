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

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <sys/timeb.h>
#include <pthread.h>
#include <stdexcept>
#include <cerrno>

namespace cpptask
{

class Mutex
{
public:
    Mutex()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        if (::pthread_mutex_init(&pmutex, &attr) != 0)
        {
            pthread_mutexattr_destroy(&attr);
            throw std::runtime_error("Can't create a mutex");
        }
        pthread_mutexattr_destroy(&attr);
    }
    ~Mutex()
    {
        ::pthread_mutex_destroy(&pmutex);
    }
    void Lock()
    {
        ::pthread_mutex_lock(&pmutex);
    }

    bool TryLock()
    {
        int err = pthread_mutex_trylock(&pmutex);
        if (err == 0)
        {
            return true;
        }
        return false;
    }

    bool WaitLock(unsigned long time)
    {
        const unsigned long NANOSEC_PER_MILLISEC = 1000000;
        timespec spec;
        timeb currSysTime;
        ftime(&currSysTime);

        spec.tv_sec = static_cast<long>(currSysTime.time);
        spec.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
        spec.tv_nsec += time * NANOSEC_PER_MILLISEC;

        if (::pthread_mutex_timedlock(&pmutex, &spec) == ETIMEDOUT)
        {
            return false;
        }
        return true;
    }

    void UnLock()
    {
        ::pthread_mutex_unlock(&pmutex);
    }

    pthread_mutex_t* GetNative()
    {
        return &pmutex;
    }
private:
    Mutex(const Mutex&);
    const Mutex& operator=(const Mutex&);
private:
    mutable pthread_mutex_t pmutex;
};

}

#endif
