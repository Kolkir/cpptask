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