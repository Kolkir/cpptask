#ifndef _THREAD_H_
#define _THREAD_H_

#include <Windows.h>
#include <process.h>
#include "exception.h"

namespace parallel
{

class Thread
{
public:
    Thread()
    {
        hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 
                                                             0, 
                                           &Thread::ThreadFunc, 
                                                          this, 
                                              CREATE_SUSPENDED, 
                                                     &threadID));
    }

    virtual ~Thread()
    {
        Wait();
        CloseHandle(hThread);
    }

    virtual void Run() = 0;
    
    void Start()
    {
        ::ResumeThread(hThread);
    }

    bool Wait() const
    {
        DWORD rez = ::WaitForSingleObject(hThread, INFINITE);
        if (rez != WAIT_OBJECT_0)
        {
            return false;
        }
        return true;
    }
    
    unsigned GetExitCode() const
    {
        DWORD code;
        ::GetExitCodeThread(hThread, &code);
        return static_cast<unsigned>(code);
    }

    const Exception& GetLastException() const
    {
        return lastException;
    }
private:
    static unsigned __stdcall ThreadFunc(void* arguments)
    {
        Thread* owner = static_cast<Thread*>(arguments);
        if (owner != 0)
        {
            return owner->ThreadFuncImpl();
        }
        return 0;
    }

    unsigned ThreadFuncImpl()
    {
        try
        {
            Run();
        }
        catch(Exception& err)
        {
            lastException = err;
            return 1;
        }
        catch(...)
        {
            return 1;
        }
        return 0;
    }
private:
    Thread(const Thread&);
    const Thread& operator=(const Thread&);
private:
    HANDLE hThread;
    unsigned threadID;
    Exception lastException;
};

}

#endif