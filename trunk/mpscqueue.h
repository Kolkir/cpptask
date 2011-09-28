#ifndef _MPSCQUEUE_H_
#define _MPSCQUEUE_H_

#include <Windows.h>
#include <intrin.h>

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

    bool IsEmpty()
    {
        if (tail == &stub)
        {
            if (tail->GetNext() == 0)
            {
                return false;
            }
        }
        return true;
    }

    void Push(MPSCNode* n)
    {        
        n->SetNext(0);        
        MPSCNode* prev = static_cast<MPSCNode*>(InterlockedExchangePointer(&head, n));
        prev->SetNext(n);
        _ReadWriteBarrier(); 
    }

    MPSCNode* Pop()
    {
        MPSCNode* locTail = tail;
        MPSCNode* next = locTail->GetNext();
        if (locTail == &stub)
        {
            if (next == 0)
            {
                return 0;
            }
            tail = next;
            locTail = next;
            next = next->GetNext();
        }
        if (next != 0)
        {
            tail = next;
            return tail;
        }
        MPSCNode* locHead = head;
        if (locTail != locHead)
        {
            return 0;
        }
        Push(&stub);
        _ReadWriteBarrier(); 
        next = locTail->GetNext();
        if (next != 0)
        {
            tail = next;
            return tail;
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