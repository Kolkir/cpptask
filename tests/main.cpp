#include "test_data.h"

#include <gtest/gtest.h>

class GlobalEnvironment : public testing::Environment 
{
public:
    virtual void SetUp() 
    {
        CppTaskTestData::instance().init();
    }

    virtual void TearDown() {}
};


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    auto genv = new GlobalEnvironment();
    if (::testing::AddGlobalTestEnvironment(genv) == genv)
    {
        return RUN_ALL_TESTS();
    }
    return -1;
}
