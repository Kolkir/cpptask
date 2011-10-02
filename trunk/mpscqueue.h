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
        //ScopedLock<Mutex> lock(&producerGuard);
        n->SetNext(0);
        MPSCNode* prev = static_cast<MPSCNode*>(InterlockedExchangePointer(&head, n));
        //MPSCNode* prev = head;
        //head = n;
        prev->SetNext(n); 
    }

    MPSCNode* Pop()
    {       
        //ScopedLock<Mutex> lock(&producerGuard);
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
            return newTail;
        }
        MPSCNode* newHead = head;
        if (newTail != newHead)
        {
            return 0;
        }
        Push(&stub);
        next = newTail->GetNext();
        if (next)
        {
            tail = next;
            return newTail;
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