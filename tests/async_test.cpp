#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

namespace
{
    void processFunc(CppTaskTestData::ArrayType& arr)
    {
        auto i = arr.begin();
        auto e = arr.end();
        for (; i != e; ++i)
        {
            CppTaskTestData::DoubleSqrt()(*i);
        }
    }
}

TEST_F(CppTaskTest, Async_Deffered)
{
    cpptask::TaskThreadPool threadPool(4);

    auto f1 = cpptask::async(std::launch::deferred, processFunc, std::ref(testArray));
    auto f2 = cpptask::async(std::launch::deferred, processFunc, std::ref(testArray2));

    ASSERT_TRUE(f1.valid());
    ASSERT_TRUE(f2.valid());

    ASSERT_FALSE(f1.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);
    ASSERT_FALSE(f2.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);

    ASSERT_NO_THROW(f1.get());
    ASSERT_NO_THROW(f2.get());

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}

TEST_F(CppTaskTest, Async_Parallel)
{
    cpptask::TaskThreadPool threadPool(4);

    auto f1 = cpptask::async(std::launch::async, processFunc, std::ref(testArray));
    auto f2 = cpptask::async(std::launch::async, processFunc, std::ref(testArray2));

    ASSERT_TRUE(f1.valid());
    ASSERT_TRUE(f2.valid());

    ASSERT_FALSE(f1.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);
    ASSERT_FALSE(f2.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);

    ASSERT_NO_THROW(f1.get());
    ASSERT_NO_THROW(f2.get());

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}

TEST_F(CppTaskTest, Async_ParallelWait)
{
    cpptask::TaskThreadPool threadPool(4);

    auto f1 = cpptask::async(std::launch::async, processFunc, std::ref(testArray));
    auto f2 = cpptask::async(std::launch::async, processFunc, std::ref(testArray2));

    ASSERT_TRUE(f1.valid());
    ASSERT_TRUE(f2.valid());

    ASSERT_FALSE(f1.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);
    ASSERT_FALSE(f2.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);

    ASSERT_NO_THROW(f1.wait());
    ASSERT_NO_THROW(f1.get());
    ASSERT_NO_THROW(f2.wait());
    ASSERT_NO_THROW(f2.get());

    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray);
    ASSERT_EQ(CppTaskTestData::instance().getResultArray(), testArray2);
}