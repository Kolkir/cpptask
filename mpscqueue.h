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
        ScopedLock<Mutex> lock(&producerGuard);
        n->SetNext(0);
        //MPSCNode* prev = static_cast<MPSCNode*>(InterlockedExchangePointer(&head, n));
        MPSCNode* prev = head;
        head = n;
        prev->SetNext(n); 
    }

    MPSCNode* Pop()
    {
        MPSCNode* next = tail->GetNext();
        if (tail == &stub)
        {
            if (next == 0)
            {
                return 0;
            }
            tail = next;
            next = next->GetNext();
        }
        if (next != 0)
        {
            tail = next;           
            return next; 
        }
        
        if (tail != head)
        {
            return 0;
        }
        Push(&stub);
        next = tail->GetNext();
        if (next != 0)
        {
            MPSCNode* prevTail = tail;
            tail = next;
            return prevTail;
        }
        return 0;       
    }

private:

    MPSCNode* head;
    MPSCNode* tail;
    MPSCNode stub;

    Mutex producerGuard;
};

}

#endif