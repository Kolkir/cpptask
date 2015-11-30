#include <gtest/gtest.h>

#include <cpptask/cpptask.h>

#include <algorithm>

TEST(ExceptionsTest, Propagation)
{
    cpptask::initializer init(4);
    auto f = cpptask::async(std::launch::async,
        []()
    {
        throw cpptask::exception("Test exception message");
    });
    ASSERT_THROW(f.get(), cpptask::exception);
}
