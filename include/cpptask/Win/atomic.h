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

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#include <windows.h>
#include <intrin.h>

#pragma intrinsic(_ReadWriteBarrier)

namespace cpptask
{

#define CppTaskMemoryFence() _ReadWriteBarrier()

class AtomicFlag
{
public:
    AtomicFlag():flag(0){}
    void Set()
    {
        ::InterlockedExchange((volatile LONG*)&flag, 1);
    }
    bool IsSet()
    {
        LONG f = ::InterlockedCompareExchange((volatile LONG*)&flag, 1, 1);
        if (f > 0)
        {
            return true;
        }
        return false;
    }
    void Reset()
    {
        ::InterlockedExchange((volatile LONG*)&flag, 0);
    }
private:
    AtomicFlag(const AtomicFlag&);
    const AtomicFlag& operator=(const AtomicFlag&);
private:
    LONG flag;
};

class AtomicNumber
{
public:
    AtomicNumber():number(0){}
    void Inc()
    {
        ::InterlockedIncrement((volatile LONG*)&number);
    }
    void Dec()
    {
        ::InterlockedDecrement((volatile LONG*)&number);
    }
    long GetValue()
    {
        LONG value = ::InterlockedCompareExchange((volatile LONG*)&number, number, number);
        return value;
    }
    void SetValue(long value)
    {
        ::InterlockedExchange((volatile LONG*)&number, value);
    }
private:
    AtomicNumber(const AtomicFlag&);
    const AtomicNumber& operator=(const AtomicFlag&);
private:
    LONG number;
};

}

#endif