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

namespace cpptask
{

class SplitMark
{
};

template<class Range, class Functor>
class ReduceTask : public Task
{
public:
    ReduceTask(const Range& range, 
               Functor& functor, 
               size_t maxDepth)
        : myDepth(maxDepth)
        , range(range)
        , functor(functor)
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
        if (myDepth > 0)
        {
            const size_t splitCount = 2;
            std::vector<Range> ranges = SplitRange(range.start, range.end, splitCount);
        
            typedef ReduceTask<Range, Functor> TASK;
            typedef RefPtr<TASK> TASKPtr;
            TASKPtr tasks[splitCount];

            typedef Functor* FUNCPtr;
            FUNCPtr functors[splitCount];
            AlignedPointer<Functor> mems[splitCount];

            TaskManager* manager = TaskManager::GetCurrent();

            for (size_t i = 0; i != splitCount; ++i)
            {
                mems[i].SetMemory(AlignedAlloc(sizeof(Functor), manager->GetCacheLineSize()));
                functors[i] = new(mems[i].GetMemory()) Functor(functor, SplitMark());
                
                TASK* ptr = new(manager->GetCacheLineSize()) TASK(ranges[i], 
                                     *functors[i], 
                                     myDepth - 1);

                tasks[i].Reset(ptr);
                manager->AddTask(tasks[i].Get());
            };

            for (size_t i = 0; i != splitCount; ++i)
            {
                manager->WaitTask(tasks[i].Get());
            };

            //check exceptions in child tasks
            for (size_t i = 0; i != splitCount; ++i)
            {
                if (tasks[i]->GetLastException() != 0)
                {
                    tasks[i]->GetLastException()->Throw();
                }
            };

            //Join
            for (size_t i = 1; i != splitCount; ++i)
            {
                tasks[0]->Join(*tasks[i].Get());
            }
            Join(*tasks[0].Get());
        }
        else
        {
            functor(range);
        }
    }

private:
    size_t myDepth;
    Range range;
    Functor& functor;
};

template<class Iterator, class Functor>
inline void ParallelReduce(Iterator start, Iterator end, Functor& functor, size_t maxDepth = 5)
{
    TaskManager* manager = TaskManager::GetCurrent();
    typedef Range<Iterator> RANGE;
    typedef ReduceTask<RANGE, Functor> TASK;
    typedef RefPtr<TASK> TASKPtr;
    TASKPtr task(new(manager->GetCacheLineSize()) TASK(RANGE(start, end), functor, maxDepth));
    manager->AddTask(task.Get());
    manager->WaitTask(task.Get());
    if (task->GetLastException() != 0)
    {
        task->GetLastException()->Throw();
    }
}

}

#endif
