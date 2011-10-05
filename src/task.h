#ifndef _TASK_H_
#define _TASK_H_

#include "thread.h"
#include "mutex.h"
#include "event.h"
#include "atomic.h"
#include "mpscqueue.h"

#include <Windows.h>

#include <algorithm>
#include <vector>

namespace parallel
{

class Task : public MPSCNode
{
public:
    Task(){waitEvent.Reset();}
    virtual ~Task(){}
    virtual void Execute() = 0;

    void Run()
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
        waitEvent.Signal();
    }

    std::shared_ptr<Exception> GetLastException();

    void Wait()
    {
        waitEvent.Wait();
        waitEvent.Reset();
    }

private:
    Task(const Task&);
    const Task& operator=(const Task&);
private:
    std::shared_ptr<Exception> lastException;
    Mutex exceptionGuard;
    Event waitEvent;
};


class TaskThread: public Thread
{
public:
    TaskThread(Event* emptyThreadEvent):task(0),emptyThreadEvent(emptyThreadEvent){}
    ~TaskThread()
    {
        Stop();
        taskEvent.Signal();
    }
    void SetTask(Task* t)
    {       
        task = t;
        hasTask.Set();
        taskEvent.Signal();
    }
    virtual void Run()
    {
        while (!done.IsSet())
        {
            taskEvent.Wait();
            if (task != 0)
            {
                task->Run();
            }
            task = 0;
            taskEvent.Reset();
            hasTask.Reset();
            if (emptyThreadEvent != 0)
            {
                emptyThreadEvent->Signal();
            }
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
    Task* task;
    Event taskEvent;
    AtomicFlag hasTask;
    Event* emptyThreadEvent;
    AtomicFlag done;
};

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
        emptyThreadEvent.Wait();
        TaskThread* rez = GetEmptyThread();
        emptyThreadEvent.Reset();
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
