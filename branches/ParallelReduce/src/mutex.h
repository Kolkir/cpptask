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

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <Windows.h>

namespace parallel
{

template <class T>
class ScopedLock
{
public:
    ScopedLock(T* guard)
        : guard(guard)
    {
        guard->Lock();
    }
    ~ScopedLock()
    {
        guard->UnLock();
    }
private:
    ScopedLock(const ScopedLock&);
    const ScopedLock& operator=(const ScopedLock&);
private:
    T* guard;
};

class Mutex
{
public:
    Mutex()
    {
        hMutex = ::CreateMutex(NULL, FALSE, NULL);
        if (hMutex == NULL)
        {
            throw std::runtime_error("Can't create a mutex");
        }
    }
    ~Mutex()
    {
        CloseHandle(hMutex);
    }
    void Lock()
    {
        DWORD rez = ::WaitForSingleObject(hMutex, INFINITE);
        if (rez != WAIT_OBJECT_0)
        {
            //log error
        }
    }

    bool WaitLock(long timeWait = INFINITE)
    {
        DWORD rez = ::WaitForSingleObject(hMutex, timeWait);
        if (rez == WAIT_OBJECT_0)
        {
            return true;
        }
        return false;
    }

    void UnLock()
    {
        BOOL rez = ::ReleaseMutex(hMutex);
        if (rez == FALSE)
        {
            //log error
        }
    }
private:
    Mutex(const Mutex&);
    const Mutex& operator=(const Mutex&);
private:
    HANDLE hMutex;
};

}

#endif