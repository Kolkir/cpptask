#ifndef _TIMER_H_
#define _TIMER_H_

#include <windows.h>

namespace parallel
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