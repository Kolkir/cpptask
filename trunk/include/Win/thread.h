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

#ifndef _THREAD_H_
#define _THREAD_H_

#include <Windows.h>
#include <process.h>
#include "exception.h"

namespace cpptask
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
        if (hThread == 0)
        {
            throw std::runtime_error("Can't create a thread");
        }
    }

    virtual ~Thread()
    {
        if (!::CloseHandle(hThread))
        {
            assert(false);
        }
    }

    virtual void Run() = 0;
    
    void Start()
    {
        if (::ResumeThread(hThread) == -1)
        {
            assert(false);
        }
    }

    void Wait() const
    {
        DWORD rez = ::WaitForSingleObject(hThread, INFINITE);
        if (rez != WAIT_OBJECT_0)
        {
            assert(false);
        }
    }
    
    unsigned long GetExitCode() const
    {
        DWORD code;
        if (!::GetExitCodeThread(hThread, &code))
        {
            assert(false);
        }
        return static_cast<unsigned long>(code);
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
        catch(std::exception& err)
        {
            lastException = Exception(err.what());
            return 1;
        }
        catch(...)
        {
            assert(false);
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

inline void Sleep(size_t ms)
{
    ::Sleep(static_cast<DWORD>(ms));
}

}

#endif