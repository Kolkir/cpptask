#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include "thread.h"
#include "mutex.h"
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
    virtual ~Task(){Wait();}
    virtual void Execute() = 0;

    void Run()
    {
        ScopedLock<Mutex> lock(&waitGuard);
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

    std::shared_ptr<Exception> GetLastException();

    void Wait()
    {
        ScopedLock<Mutex> lock(&waitGuard);
    }

private:
    Task(const Task&);
    const Task& operator=(const Task&);
private:   
    std::shared_ptr<Exception> lastException;
    Mutex exceptionGuard;
    Mutex waitGuard;
};

class TaskThread: public Thread
{
public:
    TaskThread():task(0){}
    void SetTask(Task* t){task = t;}//sync
    virtual void Run()
    {
        if (task != 0)
        {
            task->Run();
        }
        task = 0;
    }
    bool HasTask(){return task == 0;}
private:
    Task* task;
};

class ThreadPool
{
public:
    ThreadPool(size_t threadsNum);
    
    size_t GetThreadsNum() const;

    TaskThread* GetEmptyThread() const; //sync

    void PushBackThread(TaskThread*); //sync

private:
    ThreadPool(const ThreadPool&);
    const ThreadPool& operator=(const ThreadPool&);
private:
    //TaskThread s
};


template<class Iterator>
class Range
{
public:
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
    ManagerThread():done(0){}
    virtual void Run();
    void SetManager(TaskManager* manager){this->manager = manager;}
    void Finish()
    {
        ::InterlockedIncrement(&done);
    }
private:
    volatile unsigned int done;
    TaskManager* manager;
};

class TaskManager
{
public:
    TaskManager(ThreadPool& threadPool);
    ~TaskManager();

    void StartTask(Task* task);

    bool HasQueuedTasks(){return taskQueue.IsEmpty();}

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
        std::vector<RANGE> ranges(rangesNum);
        if (rangesNum > 1)
        {
            for(decltype(rangesNum) i = 0; i < rangesNum; ++i)
            {
                ranges.push_back(RANGE(start + (i * rangeLen), 
                                       start + (i * rangeLen) + 1));
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
    ThreadPool& threadPool;
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
    virtual void Execute()
    {
        for(;range.start != range.end; ++range.start)
        {
            functor(*range.start);
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
    std::vector<TASK> tasks;

    std::for_each(ranges.begin(), ranges.end(),
        [&](RANGE& range)
    {
        TASK task(range, functor);
        manager.StartTask(&task);
    });

    std::for_each(tasks.begin(), tasks.end(),
    [&](TASK& task)
    {
        task.Wait();
    });
}

}

#endif
