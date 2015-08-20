/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2011, Kyrylo Kolodiazhnyi
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
#include <type_traits>

namespace cpptask
{

class SplitMark
{
};

namespace internal
{
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
                typedef std::shared_ptr<TASK> TASKPtr;
                TASKPtr tasks[splitCount];

                typename std::aligned_storage<sizeof(Functor), _CPP_TASK_CACHE_LINE_SIZE_>::type functors[splitCount];

                auto& manager = TaskManager::GetCurrent();

                for (size_t i = 0; i != splitCount; ++i)
                {
                    new(functors + i)Functor(functor, SplitMark());

                    TASK* ptr = new TASK(ranges[i],
                        *reinterpret_cast<Functor*>(functors + i),
                        myDepth - 1);

                    tasks[i].reset(ptr);
                    manager.AddTask(*tasks[i]);
                };

                for (size_t i = 0; i != splitCount; ++i)
                {
                    manager.WaitTask(*tasks[i]);
                };

                //check exceptions in child tasks
                for (size_t i = 0; i != splitCount; ++i)
                {
                    if (tasks[i]->GetLastException() != nullptr)
                    {
                        std::rethrow_exception(tasks[i]->GetLastException());
                    }
                };

                //Join
                for (size_t i = 1; i != splitCount; ++i)
                {
                    tasks[0]->Join(*tasks[i]);
                }
                Join(*tasks[0]);
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
}

template<class Iterator, class Functor>
inline void ParallelReduce(Iterator start, Iterator end, Functor& functor, size_t maxDepth = 5)
{
    auto& manager = TaskManager::GetCurrent();

    typedef Range<Iterator> RANGE;
    typedef internal::ReduceTask<RANGE, Functor> TASK;
    TASK task(RANGE(start, end), functor, maxDepth);
    manager.AddTask(task);
    manager.WaitTask(task);
    if (task.GetLastException() != nullptr)
    {
        std::rethrow_exception(task.GetLastException());
    }
}


namespace internal
{
    template<class Functor, class Range>
    typename std::result_of<Functor(const Range&)>::type reduceFunc(Functor&& functor, const Range& range, size_t depth)
    {
        typedef std::result_of<Functor(const Range&)>::type ReturnType;
        if (depth > 0) //we can split more
        {
            const size_t splitCount = 2;
            std::vector<Range> ranges = SplitRange(range.start, range.end, splitCount);
            std::vector<future<ReturnType>> futures;
            for (size_t i = 0; i < splitCount; ++i) //put tasks to queue - they can be calculated in parallel
            {
                futures.emplace_back(cpptask::async(std::launch::async, reduceFunc, std::forward<Functor>(functor), std::cref(range), depth));
            }
            //wait results
            ReturnType result;
            for (auto& f : futures)
            {
                result += f.get();
            }
            return result;
        }
        else //stop spliting - calculate
        {
            return functor(range);
        }
    };
}

template<class Iterator, class Functor>
typename std::result_of<Functor(const Range<Iterator>&)>::type reduce(Iterator start, Iterator end, Functor&& functor, size_t maxDepth = 5)
{
    typedef Range<Iterator> RangeType;
   
    RangeType range(start, end);
    
    return internal::reduceFunc(std::forward<Functor>(functor),range,0);
    //auto f = cpptask::async(std::launch::async, internal::reduceFunc, std::forward<Functor>(functor), std::cref(range), maxDepth);
    //return f.get();
}

}

#endif
