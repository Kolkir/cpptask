#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include <algorithm>
#include <vector>

namespace parallel
{

class Exception: public std::exception
{
};

class Thread
{
public:
    virtual ~Thread(){}
    virtual void Run() = 0;
    const Exception* GetLastException() const;
};

class Task
{
public:
    virtual ~Task(){}
    virtual void Execute() = 0;

    void Wait() const;
};

class TaskThread: public Thread
{
public:
    virtual void Run()
    {
        task->Execute();
    }
private:
    Task* task;
};

class ThreadPool
{
public:
    ThreadPool(size_t threadsNum);
    
    size_t GetThreadsNum() const;

    Thread* GetEmptyThread() const; //sync

    void PushBackThread(Thread*); //sync

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

class TaskManager
{
public:
    TaskManager(ThreadPool& threadPool);

    void StartTask(Task* task);

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
void parallel_for(Iterator start, Iterator end, Functor functor, TaskManager& manager)
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
