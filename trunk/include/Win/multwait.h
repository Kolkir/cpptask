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

#ifndef _MULTWAIT_H_
#define _MULTWAIT_H_

#include <windows.h>

#include <vector>
#include <assert.h>

namespace cpptask
{

template<class E, class M>
class MultWaitBase
{
public:
    virtual ~MultWaitBase(){}
    virtual HANDLE GetHandle() = 0;
};

template<class E, class M>
int WaitForMultiple(std::vector<MultWaitBase<E, M>*>& objects)
{
    if (!objects.empty())
    {
        std::vector<HANDLE> handles;
        std::vector<MultWaitBase<E,M>*>::iterator i = objects.begin();
        std::vector<MultWaitBase<E,M>*>::iterator e = objects.end();
        for (; i != e; ++i)
        {
            handles.push_back((*i)->GetHandle());
        }        
        DWORD rez = ::WaitForMultipleObjects(static_cast<DWORD>(handles.size()), &handles[0], FALSE, INFINITE);
        if (rez >= WAIT_OBJECT_0 && rez <= WAIT_OBJECT_0 + handles.size() - 1)
        {
            return rez - WAIT_OBJECT_0;
        }
        else
        {
            assert(false);
        }
    }
    return -1;
}


}

#endif