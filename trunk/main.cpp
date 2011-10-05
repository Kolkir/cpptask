#include "parallelfor.h"
#include "timer.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

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

void SerialTest()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();
    
    timer.Start();
    std::for_each(big_array.begin(), big_array.end(),
        [](double& x)
    {
        x = std::sqrt(std::sqrt(x));
    });
    
    std::cout << "Serial time is : " << timer.End() << " ms\n";
}

void ParallelTest()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();

    parallel::TaskThreadPool threadPool(2);
    parallel::TaskManager manager(threadPool);

    big_array = GetBigArray();

    timer.Start();
    parallel::ParallelFor(big_array.begin(), big_array.end(),
        [](double& x)
    {
        x = std::sqrt(std::sqrt(x));
    }, manager);    
    
    std::cout << "Parallel time is : " << timer.End() << " ms\n";
}

int main(int /*argc*/, char* /*argv*/[])
{       
    SerialTest();

    ParallelTest();
    
    return 0;
}
