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

#include "thread.h"
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

class TaskThread;
class Task
{
public:
    friend class TaskThread;

    Task()
        : parentThread(0)
    {
        waitEvent.Reset();
    }
    virtual ~Task(){}
    virtual void Execute() = 0;

    void SetParentThread(TaskThread* thread)
    {
        parentThread = thread;
    }

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

    void WaitChildTask(Task* childTask);

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
    TaskThread* parentThread;
};


class TaskThread: public Thread
{
public:
    TaskThread(Event* emptyThreadEvent):task(0),emptyThreadEvent(emptyThreadEvent){}
    ~TaskThread()
    {
        Stop();
        taskEvent.Signal();
        Wait();
    }
    void SetTask(Task* t)
    {
        {
            ScopedLock<Mutex> lock(&guard);
            task = t;
            task->SetParentThread(this);
            hasTask.Set();
        }
        taskEvent.Signal();
    }
    void DoWaitingTasks(Task* waitTask)
    {
        //we are inside DoTask lock
        Task* oldTask = task;
        task = 0;
        hasTask.Reset();

        //clear lock to be able process another tasks
        guard.UnLock();

        taskEvent.Reset();
        if (emptyThreadEvent != 0)
        {
            emptyThreadEvent->Signal();
        }

        while (!waitTask->CheckFinished())
        {
            DoTask(&waitTask->waitEvent);
        }

        //restore lock
        guard.Lock();

        DoTaskImpl();
        task = oldTask;
        task->SetParentThread(this);
        hasTask.Set();
    }
    virtual void Run()
    {
        while (!done.IsSet())
        {
            DoTask();;
        }
    }
    bool HasTask()
    {
        return hasTask.IsSet();
    }
    void Stop()
    {
        done.Set();
    }
private:
    void DoTask(Event* secondEvent = 0)
    {
        if (secondEvent != 0)
        {
            std::vector<Event*> events(2);
            events[0] = &taskEvent;
            events[1] = secondEvent;
            WaitForMultiple(events);
        }
        else
        {
            taskEvent.Wait();
        }
        ScopedLock<Mutex> lock(&guard);
        DoTaskImpl();
    }

    void DoTaskImpl()
    {
        if (task != 0)
        {
            task->Run();
            task->SetParentThread(0);
            task->SignalDone();
            task = 0;
            hasTask.Reset();
            taskEvent.Reset();
            if (emptyThreadEvent != 0)
            {
                emptyThreadEvent->Signal();
            }
        }
    }
private:
    Task* task;
    Event* emptyThreadEvent;
    Mutex guard;
    AtomicFlag done;
    Event taskEvent;
    AtomicFlag hasTask;
};

inline void Task::WaitChildTask(Task* childTask)
{
    if (parentThread != 0)
    {
        parentThread->DoWaitingTasks(childTask);
    }
    else
    {
        childTask->Wait();
    }
}

namespace
{
struct GetEmptyThreadOp : public std::unary_function<RefPtr<TaskThread>&, bool>
{
    bool operator()(RefPtr<TaskThread>& thread) const
    {
        if (!thread->HasTask())
        {
            return true;
        }
        return false;
    }
};
}

class TaskThreadPool
{
public:
    TaskThreadPool(size_t threadsNum)
    {
        for (size_t i = 0; i < threadsNum; ++i)
        {
            TaskThreadPtr tptr(new TaskThread(&emptyThreadEvent));
            threads.push_back(tptr);
            tptr->Start();
        }
    }
    ~TaskThreadPool()
    {
        Threads::iterator i = threads.begin();
        Threads::iterator e = threads.end();
        for (; i != e; ++i)
        {
            i->Reset(0);
        }
    }

    size_t GetThreadsNum() const
    {
        return threads.size();
    }

    TaskThread* GetEmptyThread()
    {
        Threads::iterator i = std::find_if(threads.begin(), threads.end(), GetEmptyThreadOp());

        if (i != threads.end())
        {
            return i->Get();
        }
        return 0;
    }

    TaskThread* GetEmptyThreadWait()
    {
        TaskThread* rez = 0;
        while (rez == 0)
        {
            emptyThreadEvent.Wait();
            rez = GetEmptyThread();
            emptyThreadEvent.Reset();
        }
        return rez;
    }

private:
    TaskThreadPool(const TaskThreadPool&);
    const TaskThreadPool& operator=(const TaskThreadPool&);
private:
    typedef RefPtr<TaskThread> TaskThreadPtr;
    typedef std::vector<TaskThreadPtr> Threads;
    Threads threads;
    Event emptyThreadEvent;
};

}
#endif
