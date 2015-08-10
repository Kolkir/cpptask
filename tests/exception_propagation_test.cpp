#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

TEST_F(CppTaskTest, Exception_Propagation)
{
    cpptask::TaskThreadPool threadPool(4);

    ASSERT_THROW(cpptask::ParallelInvoke(
        []()
    {
        throw cpptask::Exception("Test exception message");
    }, 
        [this]()
    {
        double x = 5;
        DoubleSqrt()(x);
    }), cpptask::Exception);
}