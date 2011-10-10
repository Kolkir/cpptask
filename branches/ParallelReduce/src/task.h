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
#include "mpscqueue.h"

#include <algorithm>
#include <vector>
#include <iostream>

namespace parallel
{

class TaskThread;
class Task : public MPSCNode
{
public:
    friend class TaskThread;

    Task() 
        : parentThread(0) 
    {
        done.Reset();
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
        done.Set();
        waitEvent.Signal();
    }

    void Run()
    {     
        if (!done.IsSet())
        {
            try
            {
                Execute();
            }
            catch(Exception& err)
            {
                ScopedLock<Mutex> lock(&exceptionGuard);
                lastException.reset(err.Clone());
            }
        }
        else
        {
            DebugBreak();
        }
    }

    bool IsDone()
    {
        return done.IsSet();
    }

    std::shared_ptr<Exception> GetLastException();

    void WaitChildTask(Task* childTask);

    bool ChechFinished()
    {
        if (waitEvent.Check())
        {
            //waitEvent.Reset();
            return true;
        }
        return false;
    }

    void Wait()
    {
        waitEvent.Wait();
        //waitEvent.Reset();
    }

    void AddChild()
    {
        childrenCount.Inc();
    }
    void DelChild()
    {
        childrenCount.Dec();
    }

private:
    Task(const Task&);
    const Task& operator=(const Task&);
private:
    std::shared_ptr<Exception> lastException;
    Mutex exceptionGuard;
    Event waitEvent;
    TaskThread* parentThread;
    AtomicFlag done;
    AtomicNumber childrenCount;
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
        }
        taskEvent.Signal();
    }
    void DoWaitingTasks(Task* waitTask)
    {
        //we are inside DoTask lock
        Task* oldTask = 0;
        {
            ScopedLock<Mutex> lock(&guard);
            oldTask = task;        
            task = 0;
        }
        
        //clear lock to be able process another tasks
        guard.UnLock();

        taskEvent.Reset();                
        if (emptyThreadEvent != 0)
        {
            emptyThreadEvent->Signal();
        }

        while (!waitTask->ChechFinished())
        {
            DoTask(&waitTask->waitEvent);
        }

        //restore lock
        guard.Lock();
        SetTask(oldTask);        
    }
    virtual void Run()
    {
        while (!done.IsSet())
        {
            DoTask();
        }
    }
    bool HasTask()
    {       
        bool rez = true;
        if (guard.WaitLock(0))
        {
            rez = task != 0;
            guard.UnLock();
        }
        return rez;
    }
    void Stop()
    {
        done.Set();
    }
private:
    void DoTask(Event* secondEvent = 0)
    {        
        int eventId = -1;
        if (secondEvent != 0)
        {
            eventId = taskEvent.WaitForTwo(*secondEvent);
            if (eventId == 1)
            {
                ScopedLock<Mutex> lock(&guard);
                if (task != 0)
                {
                    eventId = 0;
                    taskEvent.Wait();
                }
            }
        }
        else
        {
            eventId = 0;
            taskEvent.Wait();
        }        
        {
            ScopedLock<Mutex> lock(&guard);
            if (task != 0)
            {
                task->Run();
                task->SetParentThread(0); 
                task->SignalDone();
                task = 0;

                if (eventId != 0)
                {
                    DebugBreak();
                }
            }
        }
        if (eventId == 0)
        {
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
    
    size_t GetThreadsNum() const
    {
        return threads.size();
    }

    TaskThread* GetEmptyThread()
    {
        Threads::iterator i =
        std::find_if(threads.begin(), threads.end(),
            [&](TaskThreadPtr& thread)->bool
        {
            if (!thread->HasTask())
            {
                return true;
            }
            return false;
        });
        if (i != threads.end())
        {
            return i->get();
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
    typedef std::shared_ptr<TaskThread> TaskThreadPtr;
    typedef std::vector<TaskThreadPtr> Threads;
    Threads threads;
    Event emptyThreadEvent;
};

}
#endif
