/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2011, Kirill Kolodyazhnyi
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "src/parallelfor.h"
#include "src/parallelinvoke.h"
#include "src/parallelreduce.h"
#include "src/timer.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

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

void SerialTest1()
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

void ParallelTest1()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();

    parallel::TaskThreadPool threadPool(THREADS_NUM);
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

void ParallelTest2()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();

    parallel::TaskThreadPool threadPool(THREADS_NUM);
    parallel::TaskManager manager(threadPool);

    big_array = GetBigArray();

    timer.Start();
    parallel::ParallelInvoke(
        [&]() {
            ArrayType::iterator i = big_array.begin();
            ArrayType::iterator e = big_array.begin();
            std::advance(e, big_array.size() / 2);
            for (;i != e; ++i)
            {
                *i = std::sqrt(std::sqrt(*i));
            }
        },
        [&]() {
            ArrayType::iterator i = big_array.begin();
            ArrayType::iterator e = big_array.end();
            std::advance(i, big_array.size() / 2);
            for (;i != e; ++i)
            {
                *i = std::sqrt(std::sqrt(*i));
            }
        },
        manager);    
    
    std::cout << "Parallel time is : " << timer.End() << " ms\n";
}

double SerialTest2()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();
    
    timer.Start();
    double res = 0;
    res = std::accumulate(big_array.begin(), big_array.end(), res, 
        [](double& x, double& y)->double
    {
        return y + x;
    });
    
    std::cout << "Serial time is : " << timer.End() << " ms\n";
    return res;
}

class Accumulator
{
public:
    Accumulator(double init) : res(init){}
    Accumulator(const Accumulator& accumulator, parallel::SplitMark) 
        : res(accumulator.res){}

    void operator()(const parallel::Range<ArrayType::iterator>& range)
    {
        std::for_each(range.start, range.end,
            [&](double x)
        {
            res += x;
        });
    }

    void Join(const Accumulator& accumulator)
    {
        res += accumulator.res;
    }

    double res;
};

double ParallelTest3()
{
    parallel::Timer timer;
    ArrayType big_array = GetBigArray();

    parallel::TaskThreadPool threadPool(THREADS_NUM);
    parallel::TaskManager manager(threadPool);

    big_array = GetBigArray();

    timer.Start();
    Accumulator accumulator(0);
    parallel::ParallelReduce(big_array.begin(), big_array.end(), accumulator, manager);    
    
    std::cout << "Parallel time is : " << timer.End() << " ms\n";
    return accumulator.res;
}

int main(int /*argc*/, char* /*argv*/[])
{       
    //SerialTest1();

    //ParallelTest1();

    //ParallelTest2();

    double r1 = SerialTest2();

    double r2 = ParallelTest3();

    std::cout << "Results are " << (r1 == r2)  << " ms\n";

    return 0;
}