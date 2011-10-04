#include "timer.h"

#include <algorithm>
#include <iostream>

class A
{
public:
    int x[11 + 16];
    A* next;
};

class B
{
public:
    int x[15 + 16];
    B* next;
};

int main(int /*argc*/, char* /*argv*/[])
{       
    using namespace parallel;    

    size_t l1 = sizeof(A);
    size_t l2 = sizeof(B);
    size_t len = 1024 * 32;

    Timer timer;
    size_t cycles = 0;

    A* m1 = new A[len];
    std::cout << "Size A = " << l1 << std::endl;
    for(size_t i = 0; i < len; ++i)
    {
        m1[i].next = 0;
        if (i > 0)
        {
            m1[i - 1].next = &m1[i];
        }
    }
    timer.Start();
    while (timer.End() < 10000)//10s
    {
        A* item = &m1[0];
        while (item != 0)
        {
            item->x[4] += 5;
            item = item->next;
        }
        ++cycles;
    }
    std::cout << "Class A cycles: " << cycles << "\n";
    delete[] m1;


    /*************************/

    cycles = 0;
    B* m2 = new B[len];
    std::cout << "Size B = " << l2 << std::endl;
    for(size_t i = 0; i < len; ++i)
    {
        m2[i].next = 0;
        if (i > 0)
        {
            m2[i - 1].next = &m2[i];
        }
    }
    timer.Start();
    while (timer.End() < 10000)//10s
    {
        B* item = &m2[0];
        while (item != 0)
        {
            item->x[4] += 5;
            item = item->next;
        }
        ++cycles;
    }
    std::cout << "Class B cycles: " << cycles << "\n";
    delete[] m2;

    return 0;
}
