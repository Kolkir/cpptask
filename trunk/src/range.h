/*
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

#ifndef _RANGE_H_
#define _RANGE_H_

#include <algorithm>
#include <vector>

namespace parallel
{

template<class Iterator>
class Range
{
public:
    typedef Iterator value_type;
    Range(){}
    ~Range(){}
    Range(Iterator start, Iterator end)
        : start(start)
        , end(end)
    {
    }
    Iterator start;
    Iterator end;
};

template<class Iterator>
std::vector<Range<Iterator>> SplitRange(Iterator start, Iterator end, size_t rangesNum)
{
    typedef Range<Iterator> RANGE;
    auto dist = std::distance(start, end);
    auto rangeLen = dist;
    if (rangesNum != 0)
    {
        rangeLen /= rangesNum;
    }
    std::vector<RANGE> ranges;
    ranges.reserve(rangesNum);
    if (rangesNum > 1)
    {
        for(decltype(rangesNum) i = 0; i < rangesNum; ++i)
        {
            ranges.push_back(RANGE(start + (i * rangeLen), 
                                    start + (i * rangeLen) + rangeLen));
        }
        ranges[rangesNum - 1].end = end;
    }
    else
    {
        ranges.push_back(RANGE(start, end));
    }
    return ranges;
}

}
#endif
