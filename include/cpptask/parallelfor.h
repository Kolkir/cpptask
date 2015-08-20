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

#ifndef _PARALLEL_FOR_H_
#define _PARALLEL_FOR_H_

#include "range.h"
#include "async.h"

#include <vector>

namespace cpptask
{

template<class Iterator, class Functor>
void for_each(Iterator start, Iterator end, Functor functor)
{
    auto& manager = TaskManager::GetCurrent();
    typedef Range<Iterator> RangeType;
    typedef std::vector<RangeType> Ranges;
    Ranges ranges = SplitRange(start, end, manager.GetThreadsNum());
    std::vector<future<void>> futures;
    for (const auto& r : ranges)
    {
        futures.emplace_back(cpptask::async(std::launch::async, [&](const RangeType& range)
        {
            typename RangeType::value_type i = range.start;
            for (; i != range.end; ++i)
            {
                functor(get_iterator_value(i));
            };
        }, std::cref(r)));
    }
    //propagate exceptions
    for (auto& f : futures)
    {
        f.get();
    }
}

}

#endif
