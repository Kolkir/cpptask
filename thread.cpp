#include "thread.h"
#include <process.h>

namespace parallel
{
/*****************************************************************************/
Thread::Thread()
{
    hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 
                                                         0, 
                                       &Thread::ThreadFunc, 
                                                      this, 
                                          CREATE_SUSPENDED, 
                                                 &threadID));
}
/*****************************************************************************/
Thread::~Thread()
{
    Wait();
    CloseHandle(hThread);
}
/*****************************************************************************/
unsigned Thread::ThreadFunc(void* arguments)
{
    Thread* owner = static_cast<Thread*>(arguments);
    if (owner != 0)
    {
        return owner->ThreadFuncImpl();
    }
    return 0;
}
/*****************************************************************************/
unsigned Thread::ThreadFuncImpl()
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
/*****************************************************************************/
void Thread::Start()
{
    ::ResumeThread(hThread);
}
/*****************************************************************************/
bool Thread::Wait() const
{
    DWORD rez = ::WaitForSingleObject(hThread, INFINITE);
    if (rez != WAIT_OBJECT_0)
    {
        return false;
    }
    return true;
}
/*****************************************************************************/
unsigned Thread::GetExitCode() const
{
    DWORD code;
    ::GetExitCodeThread(hThread, &code);
    return static_cast<unsigned>(code);
}
/*****************************************************************************/
const Exception& Thread::GetLastException() const
{
    return lastException;
}
/*****************************************************************************/
}