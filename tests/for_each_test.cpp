#include <cpptask/cpptask.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <gtest/gtest.h>

const size_t THREADS_NUM = 4;

typedef std::vector<double> ArrayType;

ArrayType GetBigArray()
{
    const size_t n = 10000000;
    ArrayType m(n);
    for (size_t i = 0; i < n; ++i)
    {
        m[i] = static_cast<double>(i);
    }
    return m;
}

struct Test1
{
    void operator()(double& x)
    {
        x = std::sqrt(std::sqrt(x));
        x = std::sqrt(std::sqrt(x));
    }
};

void SerialTest1()
{
    cpptask::Timer timer;
    ArrayType big_array = GetBigArray();

    timer.Start();
    std::for_each(big_array.begin(), big_array.end(), Test1());

    std::cout << "Serial time is : " << timer.End() << " ms\n";
}

void ParallelTest1()
{
    cpptask::Timer timer;
    ArrayType big_array = GetBigArray();

    cpptask::TaskThreadPool threadPool(THREADS_NUM);

    big_array = GetBigArray();

    timer.Start();
    cpptask::ParallelForEach(big_array.begin(), big_array.end(), Test1());

    std::cout << "Parallel for time is : " << timer.End() << " ms\n";
}

TEST(CppTaskTest, ForEach)
{
    std::cout << "------------------------\n";
    SerialTest1();

    std::cout << "------------------------\n";
    ParallelTest1();
}
