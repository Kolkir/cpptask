#include <gtest/gtest.h>

#include <cpptask/cpptask.h>

#include <algorithm>

TEST(ExceptionsTest, Propagation)
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
        x *=x;
    }), cpptask::Exception);
}