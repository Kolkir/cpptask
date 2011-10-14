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

#ifndef _REFPTR_H
#define _REFPTR_H

#include "atomic.h"
#include <stdexcept>

namespace cpptask
{

class Counter
{
public:
    Counter() {}

    void AddRef()
    {
        count.Inc();
    }

    void DelRef()
    {
        count.Dec();
    }

    long GetCount()
    {
        return count.GetValue();
    }
private:
    Counter(const Counter&);
    Counter& operator=(const Counter&);
private:
    AtomicNumber count;
};


template<class T>
class RefPtr
{
public:
    RefPtr():counter(0), pointer(0){}

    explicit RefPtr(T* ptr)
    {
        if(ptr != 0)
        {
            counter = new Counter();
            counter->AddRef();
            pointer = ptr;
        }
        else
        {
            counter = 0;
            pointer = 0;
        }
    }

    T* Reset(T* ptr)
    {
        T* oldPtr = pointer;
        Delete();
        counter = new Counter();
        counter->AddRef();
        pointer = ptr;
        return oldPtr;
    }

    RefPtr(const RefPtr& aptr)
    {
        Copy(*this, aptr);
    }

    template<class TT>
    RefPtr(const RefPtr<TT>& aptr)
    {
        Copy(*this, aptr);
    }

    ~RefPtr()
    {
        Delete();
    }

    RefPtr& operator = (const RefPtr& rhs)
    {
        if(IsEqual(*this, rhs))
        {
            return *this;
        }
        Delete();
        Copy(*this, rhs);
        return *this;
    }

    template<class TT>
    RefPtr& operator = (const RefPtr<TT>& rhs)
    {
        if(IsEqual(*this,rhs))
        {
            return *this;
        }
        Delete();
        Copy(*this, rhs);
        return *this;
    }

    T& operator*()
    {
        if(counter == 0 || pointer == 0)
        {
            throw std::logic_error("Not initialized pointer");
        }
        return *pointer;
    }

    T* operator->()
    {
        return Get();
    }

    const T& operator*() const
    {
        if(counter == 0 || pointer == 0)
        {
            throw std::logic_error("Not initialized pointer");
        }
        return *pointer;
    }

    const T* operator->()const
    {
        return Get();
    }

    T* Get()
    {
        if(counter == 0 || pointer == 0)
        {
            throw std::logic_error("Not initialized m_pointer");
        }
        return pointer;
    }

    const T* Get() const
    {
        if(counter == 0 || pointer == 0)
        {
            throw std::logic_error("Not initialized m_pointer");
        }
        return pointer;
    }

    int GetCount()const
    {
        if(counter != 0 && pointer != 0)
        {
            return counter->GetCount();
        }
        return 0;
    };

    bool IsNull()const
    {
        return (pointer == 0 || counter == 0);
    }

    void Delete()
    {
        if(counter != 0 && pointer != 0)
        {
            counter->DelRef();

            if(counter->GetCount() == 0)
            {
                delete pointer;
                delete counter;
            }

            counter = 0;
            pointer = 0;
        }
    }

    bool operator == (const RefPtr<T>& ptr)
    {
        return IsEqual(*this, ptr);
    }

    bool operator != (const RefPtr<T>& ptr)
    {
        return !IsEqual(*this, ptr);
    }

    operator bool()
    {
        return !IsNull();
    }
private:
    bool IsEqual(const RefPtr& ptr, const RefPtr& aptr)
    {
        return (ptr.counter == aptr.counter && ptr.pointer == aptr.pointer);
    }
    void Copy(RefPtr& ptr, const RefPtr& aptr)
    {
        ptr.counter = aptr.counter;
        ptr.pointer = aptr.pointer;
        if (ptr.counter != 0)
        {
            ptr.counter->AddRef();
        }
    }
private:
    Counter* counter;
    T* pointer;
};

}
#endif
