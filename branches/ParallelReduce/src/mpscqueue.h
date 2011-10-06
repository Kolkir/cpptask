/*
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

#ifndef _MPSCQUEUE_H_
#define _MPSCQUEUE_H_

#include <Windows.h>
#include <intrin.h>
#include "mutex.h"

namespace parallel
{

class MPSCNode
{
public:
    virtual ~MPSCNode(){}
    MPSCNode():next(0){}
    void SetNext(MPSCNode* n)
    {
        next = n;
    }
    MPSCNode* GetNext()
    {
        return next;
    }
private:
    MPSCNode* next;
};

class MPSCQueue 
{
public:

    MPSCQueue()
    {
        head = &stub;
        tail = &stub;
    }

    void Push(MPSCNode* n)
    {   
        n->SetNext(0);
        MPSCNode* prev = static_cast<MPSCNode*>(InterlockedExchangePointer((volatile PVOID*)&head, n));
        prev->SetNext(n); 
    }

    MPSCNode* Pop()
    {       
        MPSCNode* newTail = tail;
        MPSCNode* next = newTail->GetNext();
        if (newTail == &stub)
        {
            if (next == 0)
            {
                return 0;
            }
            tail = next;
            newTail = next;
            next = next->GetNext();
        }
        if (next != 0)
        {
            tail = next;
            return const_cast<MPSCNode*>(newTail);
        }
        volatile MPSCNode* newHead = head;
        if (newTail != newHead)
        {
            return 0;
        }
        Push(&stub);
        next = newTail->GetNext();
        if (next)
        {
            tail = next;
            return const_cast<MPSCNode*>(newTail);
        }
        return 0;       
    }

private:

    MPSCNode* head;
    MPSCNode* tail;
    MPSCNode stub;
};

}

#endif