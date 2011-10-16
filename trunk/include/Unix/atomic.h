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

namespace cpptask
{
       /*****************************************************************************************************/
        long AtomicExchange(volatile long * Target, long Value)
        {
#ifdef _MSC_VER
                return _InterlockedExchange(const_cast<long*>(Target),Value);
#else
                asm( "xchg %0, (%1)" : "+r"(Value) : "r"(Target) );
                return Value;
#endif
        }
        /*****************************************************************************************************/
        void AtomicIncrement(volatile long * Target)
        {
#ifdef _MSC_VER
                _InterlockedIncrement(const_cast<long*>(Target));
#else
                asm volatile
                (
                        "lock; incl (%0)"
                        : // No outputs
                        : "q" (Target)
                        : "cc", "memory"
                );
#endif
        }
        /*****************************************************************************************************/
        void AtomicDecrement(volatile long * Target)
        {
#ifdef _MSC_VER
                _InterlockedDecrement(const_cast<long*>(Target));
#else
        asm volatile
        (
            "lock; decl (%0)"
            : // No outputs
            : "q" (Target)
            : "cc", "memory"
        );
#endif
        }
        /*****************************************************************************************************/
        bool AtomicCompareExchange(volatile long* ptr, long oldVal, long newVal)
        {
#ifdef _MSC_VER
        return _InterlockedCompareExchange(ptr, newVal, oldVal) == oldVal;
#else
        register bool f;
        __asm__ __volatile__
        (
            "lock; cmpxchgl %%ebx, %1;"
            "setz %0;"
            : "=r"(f), "=m"(*(ptr))
            : "a"(oldVal), "b" (newVal)
            : "memory"
        );
        return f;
#endif
        }
        /*****************************************************************************************************/
        bool AtomicCompareExchange64(volatile long* ptr, long old1, unsigned int old2, long new1, unsigned int new2)
        {
#ifdef _MSC_VER
                LONGLONG Comperand = old1 | (static_cast<LONGLONG>(old2) << 32);
                LONGLONG Exchange  = new1 | (static_cast<LONGLONG>(new2) << 32);

                return _InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(ptr), Exchange, Comperand) == Comperand;
#else

                register bool f;
        __asm__ __volatile__
        (
            "lock; cmpxchg8b %1;"
            "setz %0;"
            : "=r"(f), "=m"(*(ptr))
            : "a"(old1), "b" (new1), "c" (new2), "d" (old2)
            : "memory");
                return f;
#endif
        }
        /*****************************************************************************************************/
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
        unsigned int f = ::InterlockedCompareExchange((volatile LONG*)&flag, 1, 1);
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
        unsigned long value = ::InterlockedCompareExchange((volatile LONG*)&number, number, number);
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
