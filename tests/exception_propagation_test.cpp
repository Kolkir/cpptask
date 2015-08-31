#include <gtest/gtest.h>

#include <cpptask/cpptask.h>

#include <algorithm>

TEST(ExceptionsTest, Propagation)
{
    cpptask::initialzer init(4);
    auto f = cpptask::async(std::launch::async,
        []()
    {
        throw cpptask::Exception("Test exception message");
    });
    ASSERT_THROW(f.get(), cpptask::Exception);
}
