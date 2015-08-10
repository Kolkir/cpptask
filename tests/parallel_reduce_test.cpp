#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>


namespace
{
    class Accumulator
    {
    public:
        Accumulator(double init) : res(init) {}
        Accumulator(const Accumulator& accumulator, cpptask::SplitMark)
            : res(accumulator.res) {}

        void operator()(const cpptask::Range<CppTaskTest::ArrayType::iterator>& range)
        {
            auto i = range.start;
            auto e = range.end;
            for (; i != e; ++i)
            {
                res += *i;
                double t = std::sqrt(std::sqrt(res));
                res += t - t;
            };
        }

        void Join(const Accumulator& accumulator)
        {
            res += accumulator.res;
        }

        double res;
    };
}

TEST_F(CppTaskTest, Reduce_Serial)
{
    using namespace std::placeholders;
    cpptask::TaskThreadPool threadPool(0);

    Accumulator accumulator(0);
    cpptask::ParallelReduce(testArray.begin(), testArray.end(), accumulator);
}

TEST_F(CppTaskTest, Reduce_Parallel)
{
    using namespace std::placeholders;
    cpptask::TaskThreadPool threadPool(4);

    Accumulator accumulator(0);
    cpptask::ParallelReduce(testArray.begin(), testArray.end(), accumulator);
}