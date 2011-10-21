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

#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <unistd.h>

#include "exception.h"
#include "event.h"

namespace cpptask
{

class Thread
{
public:
    Thread()
        : exitCode(0xFFFF)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        int created = pthread_create(&pthread, &attr, &Thread::ThreadFunc, this);
        pthread_attr_destroy(&attr);
        if(created != 0)
        {
            throw std::logic_error("Can't create pthread.");
        }
    }

    virtual ~Thread()
    {
    }

    virtual void Run() = 0;

    void Start()
    {
        startEvent.Signal();
    }

    bool Wait() const
    {
        if (pthread_join(pthread, 0) != 0)
        {
            return false;
        }
        return true;
    }

    unsigned long GetExitCode() const
    {
        return exitCode;
    }

    const Exception& GetLastException() const
    {
        return lastException;
    }
private:
    static void* ThreadFunc(void* arguments)
    {
        Thread* owner = static_cast<Thread*>(arguments);
        if (owner != 0)
        {
            owner->startEvent.Wait();
            owner->exitCode = owner->ThreadFuncImpl();
        }
        pthread_exit(0);
    }

    unsigned ThreadFuncImpl()
    {
        try
        {
            Run();
        }
        catch(Exception& err)
        {
            lastException = err;
            return 1;
        }
        catch(...)
        {
            return 1;
        }
        return 0;
    }
private:
    Thread(const Thread&);
    const Thread& operator=(const Thread&);
private:
    unsigned long exitCode;
    pthread_t pthread;
    Exception lastException;
    Event startEvent;
};

void Sleep(unsigned long milliseconds)
{
    if (milliseconds<1000)
    {
        usleep(milliseconds*1000);
    }
    else
    {
        sleep(milliseconds/1000);
    }
}

}

#endif
