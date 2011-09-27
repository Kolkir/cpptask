#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <Windows.h>
#include <stdexcept>

namespace parallel
{

template <class T>
class scoped_lock
{
public:
    scoped_lock(T* guard)
        : guard(guard)
    {
        guard->Lock();
    }
    ~scoped_lock()
    {
        guard->UnLock();
    }
private:
    scoped_lock(const scoped_lock&);
    const scoped_lock& operator=(const scoped_lock&);
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
    bool Lock()
    {
        DWORD rez = ::WaitForSingleObject(hMutex, INFINITE);
        if (rez != WAIT_OBJECT_0)
        {
            //log error
        }
    }
    bool UnLock()
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