#ifndef _PARALLEL_FOR_H_
#define _PARALLEL_FOR_H_

#include "taskmanager.h"
#include "range.h"

#include <algorithm>
#include <vector>

namespace parallel
{

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
    auto ranges = SplitRange(start, end, manager.GetThreadsNum());

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
        manager.AddTask(task.get());
    });
    manager.StartTasks();

    std::for_each(tasks.begin(), tasks.end(),
    [&](TASKPtr& task)
    {
        task->Wait();
    });
}

}

#endif
