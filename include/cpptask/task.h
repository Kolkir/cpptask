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

#ifndef _CPP_TASK_TASK_H_
#define _CPP_TASK_TASK_H_

#include "event.h"
#include "alignedalloc.h"
#include "eventmanager.h"
#include "exception.h"

#include <atomic>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4324 )
#endif

namespace cpptask { namespace internal {

// wait for task finished only with TaskManager::WaitTask
class alignas(_CPP_TASK_CACHE_LINE_SIZE_) Task
{
public:
    Task()
        : isFinished(false)
    {}
    
    virtual ~Task()
    {
    }

    Task(const Task&) = delete;

    const Task& operator=(const Task&) = delete;

    virtual void Execute() = 0;

    void Run(EventManager& eventManager)
    {
        if (IsFinished())
        {
            throw exception("Task already finished");
        }

        try
        {
            Execute();
        }
        catch(...)
        {
            lastException = std::current_exception();
        }
        isFinished = true;
        eventManager.notify(EventId::TaskFinishedEvent); //required for multiple waits
    }

    std::exception_ptr GetLastException() const
    {
        return lastException;
    }

    bool IsFinished()
    {
        return isFinished == true;
    }

    void* operator new(size_t size)
    {
        return aligned_alloc(_CPP_TASK_CACHE_LINE_SIZE_, size);
    }

    void operator delete(void* ptr)
    {
        free(ptr);
    }

    void operator delete(void* ptr, size_t)
    {
        free(ptr);
    }

private:
    std::exception_ptr lastException;
    std::atomic<bool> isFinished;
};

}}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
