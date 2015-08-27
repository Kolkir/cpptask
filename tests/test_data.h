#ifndef CPPTASKTEST_DATA_H
#define CPPTASKTEST_DATA_H

#include <cmath>
#include <vector>

class CppTaskTestData
{
public:
    typedef std::vector<double> ArrayType;

    const int N = 25000000;

    struct DoubleSqrt
    {
        void operator()(double& x)
        {
            x = std::sqrt(std::sqrt(x));
            x = std::sqrt(std::sqrt(x));
        }
    };

    CppTaskTestData(const CppTaskTestData&) = delete;
    CppTaskTestData& operator=(const CppTaskTestData&) = delete;

    static CppTaskTestData& instance()
    {
        static CppTaskTestData data;
        return data;
    }

    double getSum() const
    {
        return sum;
    }

    const ArrayType& getTestArray() const
    {
        return testArray;
    }

    const ArrayType& getResultArray() const
    {
        return resultArray;
    }

    void init()
    {
        for (int i = 0; i < sum; ++i)
        {
            testArray[i] = resultArray[i] = static_cast<double>(i);
            DoubleSqrt()(resultArray[i]);
            sum += resultArray[i];
        }
    }

private:

    CppTaskTestData()
        : testArray(N, 0)
        , resultArray(N, 0)
        , sum(0)
    {
    }

private:
    ArrayType testArray;
    ArrayType resultArray;
    double sum;
};

#endif // CPPTASKTEST_H

