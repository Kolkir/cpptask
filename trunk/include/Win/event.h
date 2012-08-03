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

#ifndef _EVENT_H_
#define _EVENT_H_

#include "mutex.h"
#include "multwait.h"

#include <windows.h>
#include <assert.h>

namespace cpptask
{

class Event : public MultWaitBase<Event, Mutex>
{
public:
    Event()
    {    
        // Manual reset since multiple threads can wait on this
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(hEvent == NULL)
        {
            throw std::runtime_error("Can't create event.");
        }
    }
    ~Event()
    {  
        if (!::CloseHandle(hEvent))
        {
            assert(false);
        }
    }
    void Wait()
    {
        DWORD res = ::WaitForSingleObject(hEvent, INFINITE);
        if(res != WAIT_OBJECT_0)
        {
            assert(false);
        }
    }   
    bool Check()
    {
        DWORD rez = ::WaitForSingleObject(hEvent, 0);
        if (rez == WAIT_OBJECT_0)
        {
            return true;
        }
        else if(rez != WAIT_TIMEOUT)
        {
            assert(false);
        }
        return false;
    }
    void Signal()
    {
        if (!::SetEvent(hEvent))
        {
            assert(false);
        }
    }
    void Reset()
    {
        if (!::ResetEvent(hEvent))
        {
            assert(false);
        }
    }
private:
    Event(const Event&);
    const Event& operator=(const Event&);
    virtual HANDLE GetHandle() {return hEvent;}
private:
   HANDLE hEvent;
};

}

#endif