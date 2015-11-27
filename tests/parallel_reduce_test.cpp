#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

namespace
{
    double process_func(const cpptask::range<CppTaskTestData::ArrayType::iterator>& range)
    {
        double res = 0;
        std::for_each(range.start, range.end,
            [&res](double x)
        {
            CppTaskTestData::DoubleSqrt()(x);
            res += x;
        });
        return res;
    }

    double join_func(double a, double b)
    {
        return a + b;
    }
}

TEST_F(CppTaskTest, Reduce_Serial)
{
    cpptask::initializer init(0);

    double sum = cpptask::reduce<double>(testArray.begin(), testArray.end(), process_func, join_func);

    ASSERT_EQ(CppTaskTestData::instance().getSum(), sum);
}


TEST_F(CppTaskTest, Reduce_Parallel)
{
    cpptask::initializer init(4);

    double sum = cpptask::reduce<double>(testArray.begin(), testArray.end(), process_func, join_func);

    ASSERT_EQ(CppTaskTestData::instance().getSum(), sum);
}
