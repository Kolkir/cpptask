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

#ifndef _TASK_H_
#define _TASK_H_

#include "taskthread.h"
#include "mutex.h"
#include "event.h"
#include "atomic.h"
#include "alignedalloc.h"
#include "refptr.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace cpptask
{

class Task
{
public:
    Task()
    {
        waitEvent.Reset();
    }
    virtual ~Task(){}
    virtual void Execute() = 0;

    void SignalDone()
    {
        waitEvent.Signal();
    }

    void Run()
    {
        try
        {
            Execute();
        }
        catch(Exception& err)
        {
            ScopedLock<Mutex> lock(&exceptionGuard);
            lastException.Reset(err.Clone());
        }
        catch(...)
        {
            lastException.Reset(new Exception("Unknown exception"));
        }
    }

    const Exception* GetLastException() const
    {
        if (lastException.IsNull())
        {
            return 0;
        }
        return lastException.Get();
    }

    bool CheckFinished()
    {
        if (waitEvent.Check())
        {
            return true;
        }
        return false;
    }

    void Wait()
    {
        waitEvent.Wait();
    }

    Event* GetWaitEvent()
    {
        return &waitEvent;
    }

    void* operator new(size_t size, size_t alignment)
    {
        return AlignedAlloc(size, alignment);
    }

    void operator delete(void* ptr)
    {
        AlignedFree(ptr);
    }

    void operator delete(void* ptr, size_t)
    {
        AlignedFree(ptr);
    }

private:
    Task(const Task&);
    const Task& operator=(const Task&);
private:
    RefPtr<Exception> lastException;
    Mutex exceptionGuard;
    Event waitEvent;
};

}
#endif
