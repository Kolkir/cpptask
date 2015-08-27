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

namespace cpptask
{

namespace internal
{
    template<class ReturnType, class Range, class ProcessFunction, class JoinFunction>
    ReturnType reduceFunc(ProcessFunction&& process, JoinFunction&& join, const Range& range, size_t depth, size_t width)
    {
        if (depth > 0) //we can split more
        {
            assert(width >= 2);
            const size_t splitCount = (std::max)(size_t(2), width);
            std::vector<Range> ranges = SplitRange(range.start, range.end, splitCount);
            std::vector<future<ReturnType>> futures;
            for (size_t i = 0; i < splitCount; ++i) //put tasks to queue - they can be calculated in parallel
            {
                futures.emplace_back(cpptask::async(std::launch::async,
                    [&, i]()
                {
                    return reduceFunc<ReturnType, Range, ProcessFunction, JoinFunction>(std::forward<ProcessFunction>(process), std::forward<JoinFunction>(join), ranges[i], depth - 1, width);
                }));
            }
            //wait results
            ReturnType result = futures.front().get();
            std::for_each(std::next(futures.begin()), futures.end(),
                [&](future<ReturnType>& f)
            {
                result = join(result, f.get());
            });
            return result;
        }
        else //stop spliting - calculate
        {
            return process(range);
        }
    };
}

template<class ReturnType, class Iterator, class ProcessFunction, class JoinFunction>
ReturnType reduce(Iterator start, Iterator end, ProcessFunction&& process, JoinFunction&& join, size_t depth = 5, size_t width = 2)
{
    typedef Range<Iterator> RangeType;

    RangeType range(start, end);

    auto f = cpptask::async(std::launch::async,
        [&]()
    {
        return internal::reduceFunc<ReturnType>(std::forward<ProcessFunction>(process), std::forward<JoinFunction>(join), range, depth, width);
    });
    return f.get();
}

}

#endif
