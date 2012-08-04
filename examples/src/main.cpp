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

#include <cpptask.h>

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
    cpptask::ParallelFor(big_array.begin(), big_array.end(), Test1());

    std::cout << "Parallel for time is : " << timer.End() << " ms\n";
}

struct Test21
{
    Test21(ArrayType* big_array) : big_array(big_array){}
    void operator()()
    {
        ArrayType::iterator i = big_array->begin();
        ArrayType::iterator e = big_array->begin();
        std::advance(e, big_array->size() / 2);
        for (;i != e; ++i)
        {
            Test1()(*i);
        }
    }
    ArrayType* big_array;
};
struct Test22
{
    Test22(ArrayType* big_array) : big_array(big_array){}
    void operator()()
    {
        ArrayType::iterator i = big_array->begin();
        ArrayType::iterator e = big_array->end();
        std::advance(i, big_array->size() / 2);
        for (;i != e; ++i)
        {
            Test1()(*i);
        }
    }
    ArrayType* big_array;
};

void ParallelTest2()
{
    cpptask::Timer timer;
    ArrayType big_array = GetBigArray();

    cpptask::TaskThreadPool threadPool(THREADS_NUM);

    big_array = GetBigArray();

    timer.Start();
    cpptask::ParallelInvoke(Test21(&big_array), Test22(&big_array));

    std::cout << "Parallel invoke time is : " << timer.End() << " ms\n";
}

struct Test3
{
    double operator()(double& x, double& y)
    {
        double rez = y + x;
        double t = std::sqrt(std::sqrt(rez));
        return rez + t - t;
    }
};

double SerialTest2()
{
    cpptask::Timer timer;
    ArrayType big_array = GetBigArray();

    timer.Start();
    double res = 0;
    res = std::accumulate(big_array.begin(), big_array.end(), res, Test3());

    std::cout << "Serial time is : " << timer.End() << " ms\n";
    return res;
}

class Accumulator
{
public:
    Accumulator(double init) : res(init){}
    Accumulator(const Accumulator& accumulator, cpptask::SplitMark)
        : res(accumulator.res){}

    void operator()(const cpptask::Range<ArrayType::iterator>& range)
    {
        ArrayType::iterator i = range.start;
        ArrayType::iterator e = range.end;
        for(; i != e; ++i)
        {
            res += *i;
            double t = std::sqrt(std::sqrt(res));
            res += t - t;
        };
    }

    void Join(const Accumulator& accumulator)
    {
        res += accumulator.res;
    }

    double res;
};

double ParallelTest3()
{
    cpptask::Timer timer;
    ArrayType big_array = GetBigArray();

    cpptask::TaskThreadPool threadPool(THREADS_NUM);

    big_array = GetBigArray();

    timer.Start();
    Accumulator accumulator(0);
    cpptask::ParallelReduce(big_array.begin(), big_array.end(), accumulator);

    std::cout << "Parallel reduce time is : " << timer.End() << " ms\n";
    return accumulator.res;
}

void Test4()
{
    throw cpptask::Exception("Test exception message");
}

void ExceptionTest()
{
    try
    {
        cpptask::TaskThreadPool threadPool(THREADS_NUM);

        cpptask::ParallelInvoke(&Test4, &Test4);
    }
    catch(cpptask::Exception& err)
    {
        std::cout << "Exception in task : " << err.what() << "\n";
    }
};

typedef void (*ThreadFunc)(void);

void ThreadFunc1()
{
    for (int i = 0; i < 100; ++i)
    {
        std::cout << "Hello 1\n";
        cpptask::Sleep(10);
    }
}

void ThreadFunc2()
{
    for (int i = 0; i < 100; ++i)
    {
        std::cout << "Hello 2\n";
        cpptask::Sleep(10);
    }
}

int main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "Start parallel execution:";
    cpptask::ThreadFunction<ThreadFunc> t1(&ThreadFunc1);
    cpptask::ThreadFunction<ThreadFunc> t2(&ThreadFunc2);
    t1.Start();
    t2.Start();
    t1.Wait();
    t2.Wait();

    std::cout << "------------------------\n";
    SerialTest1();

    std::cout << "------------------------\n";
    ParallelTest1();

    std::cout << "------------------------\n";
    ParallelTest2();

    std::cout << "------------------------\n";
    double r1 = SerialTest2();

    std::cout << "------------------------\n";
    double r2 = ParallelTest3();

    std::cout << "------------------------\n";
    std::cout << "Reduce results compare are " << (r1 == r2)  << "\n";

    std::cout << "------------------------\n";
    ExceptionTest();

    std::cout << "------------------------\n";
    return 0;
}
