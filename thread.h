#ifndef _THREAD_H_
#define _THREAD_H_

#include <Windows.h>
#include "exception.h"

namespace parallel
{

class Thread
{
public:
    Thread();
    virtual ~Thread();
    virtual void Run() = 0;
    void Start();
    bool Wait() const;
    unsigned GetExitCode() const;
    const Exception& GetLastException() const;
private:
    static unsigned __stdcall ThreadFunc(void* arguments);
    unsigned ThreadFuncImpl();
    Thread(const Thread&);
    const Thread& operator=(const Thread&);
private:
    HANDLE hThread;
    unsigned threadID;
    Exception lastException;
};

}

#endif