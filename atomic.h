#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <windows.h>

namespace parallel
{

class AtomicFlag
{
public:
    AtomicFlag():flag(0){}
    void Set()
    {
        ::InterlockedIncrement(&flag);
    }
    bool IsSet()
    {
        unsigned int f = InterlockedCompareExchange(&flag, 1, 1);
        if (f == 1)
        {
            return true;
        }
        return false;
    }
    void Reset()
    {
        ::InterlockedDecrement(&flag);
    }
private:
    AtomicFlag(const AtomicFlag&);
    const AtomicFlag& operator=(const AtomicFlag&);
private:
    unsigned int flag;
};

}

#endif