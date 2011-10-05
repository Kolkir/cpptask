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
