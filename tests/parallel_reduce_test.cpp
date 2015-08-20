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

        void operator()(const cpptask::Range<CppTaskTestData::ArrayType::iterator>& range)
        {
            auto i = range.start;
            auto e = range.end;
            for (; i != e; ++i)
            {
                double t = *i;
                CppTaskTestData::DoubleSqrt()(t);
                res += t;
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
    cpptask::TaskThreadPool threadPool(0);

    Accumulator accumulator(0);
    ASSERT_NO_THROW(cpptask::ParallelReduce(testArray.begin(), testArray.end(), accumulator));

    ASSERT_EQ(CppTaskTestData::instance().getSum(), accumulator.res);
}

TEST_F(CppTaskTest, Reduce_Parallel)
{
    cpptask::TaskThreadPool threadPool(4);

    Accumulator accumulator(0);
    ASSERT_NO_THROW(cpptask::ParallelReduce(testArray.begin(), testArray.end(), accumulator));

    ASSERT_EQ(CppTaskTestData::instance().getSum(), accumulator.res);
}

namespace
{
    double reduce_func(const cpptask::Range<CppTaskTestData::ArrayType::iterator>& range)
    {
        double res = 0;
        std::for_each(range.start, range.end,
            [&res](double x)
        {
            double t = x;
            CppTaskTestData::DoubleSqrt()(t);
            res += t;
        });
        return res;
    }
}

/*
TEST_F(CppTaskTest, Reduce_Parallel2)
{
    cpptask::TaskThreadPool threadPool(4);

    double sum = cpptask::reduce(testArray.begin(), testArray.end(), reduce_func);
        

    ASSERT_EQ(CppTaskTestData::instance().getSum(), sum);
}
*/