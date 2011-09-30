#ifndef _PARALLEL_H_
#define _PARALLEL_H_

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
    Task(){}
    virtual ~Task(){}
    virtual void Execute() = 0;

    void Run()
    {        
        waitEvent.Reset();
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
        taskEvent.Wait();
        if (task != 0)
        {
            task->Run();
        }
        task = 0;
        hasTask.Reset();
        if (emptyThreadEvent != 0)
        {
            emptyThreadEvent->Signal();
        }
    }
    bool HasTask()
    {        
        return hasTask.IsSet();
        
    }
private:
    Task* task;
    Event taskEvent;
    AtomicFlag hasTask;
    Event* emptyThreadEvent;
};

class TaskThreadPool
{
public:
    TaskThreadPool(size_t threadsNum);
    
    size_t GetThreadsNum() const;

    TaskThread* GetEmptyThread();
    TaskThread* GetEmptyThreadWait();

private:
    TaskThreadPool(const TaskThreadPool&);
    const TaskThreadPool& operator=(const TaskThreadPool&);
private:
    typedef std::shared_ptr<TaskThread> TaskThreadPtr;
    typedef std::vector<TaskThreadPtr> Threads;
    Threads threads;
    Event emptyThreadEvent;
};


template<class Iterator>
class Range
{
public:
    typedef Iterator value_type;
    Range(){}
    Range(Iterator start, Iterator end)
        : start(start)
        , end(end)
    {
    }
    Iterator start;
    Iterator end;
};

class TaskManager;
class ManagerThread: public Thread
{
public:
    ManagerThread():manager(0){}
    virtual void Run();
    void SetManager(TaskManager* manager){this->manager = manager;}
    void Finish()
    {
        taskProcessEvent.Signal();
        done.Set();
    }
    void NotifyScheduleTasks()
    {
        taskProcessEvent.Signal();
    }
private:
    Event taskProcessEvent;
    AtomicFlag done;
    TaskManager* manager;
};

class TaskManager
{
public:
    TaskManager(TaskThreadPool& threadPool);
    ~TaskManager();

    void StartTask(Task* task);

    void ScheduleTasks();

    template<class Iterator>
    std::vector<Range<Iterator>> SplitRange(Iterator start, Iterator end) const
    {
        typedef Range<Iterator> RANGE;
        auto rangesNum = threadPool.GetThreadsNum();
        auto dist = std::distance(start, end);
        auto rangeLen = dist;
        if (rangesNum != 0)
        {
            rangeLen /= rangesNum;
        }
        std::vector<RANGE> ranges;
        ranges.reserve(rangesNum);
        if (rangesNum > 1)
        {
            for(decltype(rangesNum) i = 0; i < rangesNum; ++i)
            {
                ranges.push_back(RANGE(start + (i * rangeLen), 
                                       start + (i * rangeLen) + rangeLen));
            }
            ranges[rangesNum - 1].end = end;
        }
        else
        {
            ranges.push_back(RANGE(start, end));
        }
        return ranges;
    }

private:
    TaskThreadPool& threadPool;
    MPSCQueue taskQueue;
    ManagerThread managerThread;
};

template<class Range, class Functor>
class ForTask : public Task
{
public:
    ForTask(const Range& range, const Functor& functor)
        : range(range)
        , functor(functor)
    {
    }
    ~ForTask()
    {
    }
    virtual void Execute()
    {
        Range::value_type i = range.start;
        for(;i !=  range.end; ++i)
        {
            functor(*i);
        };
    }

private:
    Range range;
    Functor functor;
};

template<class Iterator, class Functor>
void ParallelFor(Iterator start, Iterator end, Functor functor, TaskManager& manager)
{        
    auto ranges = manager.SplitRange(start, end);

    typedef Range<Iterator> RANGE;
    typedef ForTask<RANGE, Functor> TASK;
    typedef std::shared_ptr<TASK> TASKPtr;
    std::vector<TASKPtr> tasks;

    std::for_each(ranges.begin(), ranges.end(),
        [&](RANGE& range)
    {
        TASK* ptr = new TASK(range, functor);
        TASKPtr task(ptr);
        tasks.push_back(task);
        manager.StartTask(task.get());
    });

    std::for_each(tasks.begin(), tasks.end(),
    [&](TASKPtr& task)
    {
        task->Wait();
    });
}

}

#endif
