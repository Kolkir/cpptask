#include "parallel_for.h"
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
        m[i] = i;
    }
    return m;
}

int main(int /*argc*/, char* /*argv*/[])
{       
    using namespace parallel;
    Timer timer;

    // serial test
    ArrayType big_array = GetBigArray();
    
    timer.Start();
    std::for_each(big_array.begin(), big_array.end(),
        [](double& x)
    {
        x = std::sqrt(std::sqrt(x));
    });
    
    std::cout << "Serial time is : " << timer.End() << " ms\n";

    // parallel test
    parallel::ThreadPool threadPool(4);
    parallel::TaskManager manager(threadPool);

    big_array = GetBigArray();

    timer.Start();
    parallel::ParallelFor(big_array.begin(), big_array.end(),
        [](double& x)
    {
        x = std::sqrt(std::sqrt(x));
    }, manager);    
    
    std::cout << "Parallel time is : " << timer.End() << " ms\n";

    return 0;
}
