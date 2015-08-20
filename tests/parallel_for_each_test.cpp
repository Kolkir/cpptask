#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

TEST_F(CppTaskTest, ForEach_Serial)
{
    cpptask::TaskThreadPool threadPool(0);

    ASSERT_NO_THROW(cpptask::for_each(testArray.begin(), testArray.end(), CppTaskTestData::DoubleSqrt()));

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
}

TEST_F(CppTaskTest, ForEach_Parallel)
{
    cpptask::TaskThreadPool threadPool(4);

    ASSERT_NO_THROW(cpptask::for_each(testArray.begin(), testArray.end(), CppTaskTestData::DoubleSqrt()));

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
}

