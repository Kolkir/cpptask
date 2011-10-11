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
               Functor& functor, 
               size_t maxDepth,
               TaskManager* manager)
        : maxDepth(maxDepth)
        , myDepth(maxDepth)
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
        if (myDepth > 0)
        {
            const size_t splitCount = 2;
            --myDepth;            
            auto ranges = SplitRange(range.start, range.end, splitCount);
        
            typedef ReduceTask<Range, Functor> TASK;
            typedef std::shared_ptr<TASK> TASKPtr;
            TASKPtr tasks[splitCount];            

            typedef Functor* FUNCPtr;            
            FUNCPtr functors[splitCount];
            void* mems[splitCount];

            size_t index = 0;
            std::for_each(ranges.begin(), ranges.end(),
                [&](Range& range)
            {
                mems[index] = AlignedAlloc(sizeof(Functor), manager->GetCacheLineSize());               
                functors[index] = new(mems[index]) Functor(functor, SplitMark());
                
                TASK* ptr = new(manager->GetCacheLineSize()) TASK(range, 
                                     *functors[index], 
                                     myDepth, 
                                     manager);          

                tasks[index].reset(ptr);
                manager->AddTask(tasks[index].get());
                ++index;
            });
            manager->StartTasks();

            std::for_each(tasks, tasks + splitCount,
            [&](TASKPtr& task)
            {
                WaitChildTask(task.get());                
            });

            //Join                        
            for (size_t i = 1; i != splitCount; ++i)
            {
                tasks[0]->Join(*tasks[i].get());
            }
            Join(*tasks[0].get());
            
            //free memory
            for (size_t i = 0; i != splitCount; ++i)
            {
                functors[i]->~Functor();
                AlignedFree(mems[i]);
            }
        }
        else
        {
            functor(range);
        }
    }

private:
    size_t maxDepth;
    size_t myDepth;
    Range range;
    Functor& functor;
    TaskManager* manager;
};

template<class Iterator, class Functor>
void ParallelReduce(Iterator start, Iterator end, Functor& functor, TaskManager& manager, size_t maxDepth = 5)
{              
    typedef Range<Iterator> RANGE;
    typedef ReduceTask<RANGE, Functor> TASK;
    typedef std::shared_ptr<TASK> TASKPtr;
    TASKPtr task(new(manager.GetCacheLineSize()) TASK(RANGE(start, end), functor, maxDepth, &manager));
    manager.AddTask(task.get());
    manager.StartTasks();
    task->Wait();
}

}

#endif
