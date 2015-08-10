#ifndef CPPTASKTEST_H
#define CPPTASKTEST_H

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

class CppTaskTest : public ::testing::Test
{
protected:

    typedef std::vector<double> ArrayType;
    ArrayType testArray;
    virtual void SetUp()
    {
        testArray = GetBigArray();
    }

    virtual void TearDown()
    {
    }

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

    struct DoubleSqrt
    {
        void operator()(double& x)
        {
            x = std::sqrt(std::sqrt(x));
            x = std::sqrt(std::sqrt(x));
        }
    };
};

#endif // CPPTASKTEST_H

