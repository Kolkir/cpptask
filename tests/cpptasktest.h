#ifndef CPPTASKTEST_H
#define CPPTASKTEST_H

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

class CppTaskTest : public ::testing::Test
{
public:
	typedef std::vector<double> ArrayType;

	struct DoubleSqrt
	{
		void operator()(double& x)
		{
			x = std::sqrt(std::sqrt(x));
			x = std::sqrt(std::sqrt(x));
		}
	};

protected:
    ArrayType testArray;
    ArrayType testArray2;

    virtual void SetUp()
    {
        testArray = GetBigArray();
        testArray2 = testArray;
    }

    virtual void TearDown()
    {
    }

    ArrayType GetBigArray()
    {
        const size_t n = 25000000;
        ArrayType m(n);
        for (size_t i = 0; i < n; ++i)
        {
            m[i] = static_cast<double>(i);
        }
        return m;
    }
};

#endif // CPPTASKTEST_H

