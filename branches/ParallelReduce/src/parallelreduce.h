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

#ifndef _PARALLEL_REDUCE_H_
#define _PARALLEL_REDUCE_H_

#include "taskmanager.h"
#include "range.h"

#include <algorithm>
#include <vector>

namespace parallel
{

class SplitMark
{
};

template<class Range, class Functor>
class ReduceTask : public Task
{
public:
    ReduceTask(const Range& range, 
               const Functor& functor, 
               size_t minRangeLength,
               TaskManager* manager)
        : minRangeLength(minRangeLength)
        , range(range)
        , functor(functor)
        , manager(manager)
    {
    }
    ~ReduceTask()
    {
    }

    void Join(ReduceTask& task)
    {
        functor.Join(task.functor);
    }

    virtual void Execute()
    {
        if (range.Size() > minRangeLength)
        {
            auto ranges = SplitRange(range.start, range.end, 2);
        
            typedef ReduceTask<Range, Functor> TASK;
            typedef std::shared_ptr<TASK> TASKPtr;
            typedef std::vector<TASKPtr> TASKS;
            TASKS tasks;

            std::for_each(ranges.begin(), ranges.end(),
                [&](Range& range)
            {
                TASK* ptr = new TASK(range, 
                                     Functor(functor, SplitMark()), 
                                     minRangeLength, 
                                     manager);
                TASKPtr task(ptr);
                tasks.push_back(task);
                manager->AddTask(task.get());
            });
            manager->StartTasks();

            std::for_each(tasks.begin(), tasks.end(),
            [&](TASKPtr& task)
            {
                task->Wait();
            });

            //Join
            TASKS::iterator first = tasks.begin();
            TASKS::iterator i = tasks.begin();
            ++i;
            TASKS::iterator e = tasks.end();
            for (;i != e; ++i)
            {
                (*first)->Join(*i->get());
            }
        }
        else
        {
            functor(range);
        }
    }

private:
    size_t minRangeLength;
    Range range;
    Functor functor;
    TaskManager* manager;
};

template<class Iterator, class Functor>
void ParallelReduce(Iterator start, Iterator end, Functor functor, TaskManager& manager, size_t minRangeLength = 100)
{              
    typedef Range<Iterator> RANGE;
    typedef ReduceTask<RANGE, Functor> TASK;
    typedef std::shared_ptr<TASK> TASKPtr;
    TASKPtr task(new TASK(RANGE(start, end), functor, minRangeLength, &manager));
    manager.AddTask(task.get());
    manager.StartTasks();
    task->Wait();
}

}

#endif
