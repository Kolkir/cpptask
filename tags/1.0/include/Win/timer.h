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

#ifndef _TIMER_H_
#define _TIMER_H_

#include <windows.h>

namespace cpptask
{
class Timer
{
public:
    Timer()
    {
         threadHandle = ::GetCurrentThread();
        ::GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemMask);
        ::SetThreadAffinityMask(threadHandle, 1);
        ::QueryPerformanceFrequency(&frequency);
        ::SetThreadAffinityMask(threadHandle, processAffinityMask);
    }
    void Start()
    {
        ::SetThreadAffinityMask(threadHandle, 1);
        ::QueryPerformanceCounter(&startTime);
        ::SetThreadAffinityMask(threadHandle, processAffinityMask);
    }
    double End()
    {
        ::SetThreadAffinityMask(threadHandle, 1);
        ::QueryPerformanceCounter(&endTime);
        ::SetThreadAffinityMask(threadHandle, processAffinityMask);

         __int64 elapsedTime = endTime.QuadPart - startTime.QuadPart;
        double const mseconds = double(elapsedTime) / (double(frequency.QuadPart) / 1000.0);
        return mseconds;
    }
private:
    Timer(const Timer&);
    const Timer& operator=(const Timer&);
private:
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
	LARGE_INTEGER frequency;
    void* threadHandle;
#if defined(_WIN64)
    unsigned __int64 processAffinityMask;
#else
    unsigned long processAffinityMask;
#endif
    DWORD_PTR systemMask;
};
}

#endif