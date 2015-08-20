#ifndef CPPTASKTEST_H
#define CPPTASKTEST_H

#include "test_data.h"
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

class CppTaskTest : public ::testing::Test
{
protected:
    CppTaskTestData::ArrayType testArray;
    char pad[64];
    CppTaskTestData::ArrayType testArray2;

    virtual void SetUp()
    {
        testArray = CppTaskTestData::instance().getTestArray();
        testArray2 = CppTaskTestData::instance().getTestArray();
    }

    virtual void TearDown()
    {
    }
};

#endif // CPPTASKTEST_H

