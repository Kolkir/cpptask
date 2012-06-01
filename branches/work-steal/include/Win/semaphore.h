/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2012, Kirill Kolodyazhnyi
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

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "multwait.h"

#include <Windows.h>
#include <stdexcept>

namespace cpptask
{

class Semaphore : public MultWaitBase
{
public:
    Semaphore(long maxCount)
    {
        hSemaphore = ::CreateSemaphore(0, 0, maxCount, 0);
        if (hSemaphore == NULL)
        {
            throw std::runtime_error("Can't create a semaphore");
        }
    }
    ~Semaphore()
    {
        CloseHandle(hSemaphore);
    }

    void Wait()
    {
        DWORD rez = ::WaitForSingleObject(hSemaphore, INFINITE);
        if (rez != WAIT_OBJECT_0)
        {
            //log error
        }
    }

    void Signal(long count)
    {
        long prevCount(0);
        BOOL rez = ::ReleaseSemaphore(hSemaphore, count, &prevCount);
        if (rez == FALSE)
        {
            //log error
        }
    }
private:
    Semaphore(const Semaphore&);
    const Semaphore& operator=(const Semaphore&);
    virtual HANDLE GetHandle() {return hSemaphore;}
private:
    HANDLE hSemaphore;
};

}

#endif
