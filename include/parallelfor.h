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

namespace cpptask
{

template<class Range, class Functor>
class ForEachTask : public Task
{
public:
    ForEachTask(const Range& range, const Functor& functor)
        : range(range)
        , functor(functor)
    {
    }
    ~ForEachTask()
    {
    }
    virtual void Execute()
    {
        typename Range::value_type i = range.start;
        for(;i !=  range.end; ++i)
        {
            functor(*i);
        };
    }

private:
    Range range;
    Functor functor;
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
        typename Range::value_type i = range.start;
        for(;i !=  range.end; ++i)
        {
            functor(i);
        };
    }

private:
    Range range;
    Functor functor;
};

template<class Functor, class RANGES, class TaskType>
void ParallelForBase(RANGES ranges, Functor functor, TaskManager* manager)
{
    typedef RefPtr<TaskType> TASKPtr;
    typedef std::vector<TASKPtr> TASKS;
    TASKS tasks;

    typename RANGES::iterator i = ranges.begin();
    typename RANGES::iterator e = ranges.end();
    for (; i != e; ++i)
    {
        TaskType* ptr = new(manager->GetCacheLineSize()) TaskType(*i, functor);
        TASKPtr task(ptr);
        tasks.push_back(task);
        manager->AddTask(task.Get());
    }

    typename TASKS::iterator it = tasks.begin();
    typename TASKS::iterator et = tasks.end();
    for (; it != et; ++it)
    {
        manager->WaitTask((*it).Get());
    }
    it = tasks.begin();
    for (; it != et; ++it)
    {
        if ((*it)->GetLastException() != 0)
        {
            (*it)->GetLastException()->Throw();
        }
    }
}

template<class Iterator, class Functor>
void ParallelForEach(Iterator start, Iterator end, Functor functor)
{
    TaskManager* manager = TaskManager::GetCurrent();
    typedef std::vector<Range<Iterator> > RANGES;
    RANGES ranges = SplitRange(start, end, manager->GetThreadsNum());
    ParallelForBase<Functor, RANGES, ForEachTask<Range<Iterator>,Functor> >(ranges, functor, manager);
}

template<class Iterator, class Functor>
void ParallelFor(Iterator start, Iterator end, Functor functor)
{
    assert(end >= start);
    TaskManager* manager = TaskManager::GetCurrent();
    typedef std::vector<Range<Iterator> > RANGES;
    RANGES ranges = SplitNumRange(start, end, manager->GetThreadsNum());
    ParallelForBase<Functor, RANGES, ForTask<Range<Iterator>,Functor> >(ranges, functor, manager);
}


}

#endif
