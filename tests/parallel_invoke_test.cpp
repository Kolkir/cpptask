#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

namespace
{
    void processFunc(CppTaskTest::ArrayType& arr)
    {
        auto i = arr.begin();
        auto e = arr.end();
        for (; i != e; ++i)
        {
            CppTaskTest::DoubleSqrt()(*i);
        }
    }
}

TEST_F(CppTaskTest, Invoke_Serial)
{
    using namespace std::placeholders;
    cpptask::TaskThreadPool threadPool(0);

    cpptask::ParallelInvoke(std::bind(processFunc, std::ref(testArray)),
                            std::bind(processFunc, std::ref(testArray2)));
}

TEST_F(CppTaskTest, Invoke_Parallel)
{
    using namespace std::placeholders;
    cpptask::TaskThreadPool threadPool(4);

    cpptask::ParallelInvoke(std::bind(processFunc, std::ref(testArray)),
                            std::bind(processFunc, std::ref(testArray2)));
}