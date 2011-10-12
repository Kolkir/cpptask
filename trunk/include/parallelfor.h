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
inline void ParallelFor(Iterator start, Iterator end, Functor functor, TaskManager& manager)
{        
    std::vector<Range<Iterator>> ranges = SplitRange(start, end, manager.GetThreadsNum());

    typedef Range<Iterator> RANGE;
    typedef ForTask<RANGE, Functor> TASK;
    typedef RefPtr<TASK> TASKPtr;
    typedef std::vector<TASKPtr> TASKS;
    TASKS tasks;

    struct AddTask
    {
        AddTask(TaskManager* manager, TASKS* tasks, Functor* functor)
            : manager(manager)
            , tasks(tasks)
            , functor(functor)
        {}
        void operator()(RANGE& range)
        {
            TASK* ptr = new(manager->GetCacheLineSize()) TASK(range, *functor);
            TASKPtr task(ptr);
            tasks->push_back(task);
            manager->AddTask(task.Get());
        }
        TaskManager* manager;
        TASKS* tasks;
        Functor* functor;
    };

    std::for_each(ranges.begin(), ranges.end(), AddTask(&manager, &tasks, &functor));
    manager.StartTasks();
    
    struct WaitTask
    {
        void operator()(TASKPtr& task)
        {
            task->Wait();
        }
    };
    std::for_each(tasks.begin(), tasks.end(), WaitTask());

    struct CheckTaskException
    {
        void operator()(TASKPtr& task)
        {
            if (task->GetLastException() != 0)
            {
                task->GetLastException()->Throw();
            }
        }
    };
    std::for_each(tasks.begin(), tasks.end(), CheckTaskException());
}

}

#endif
