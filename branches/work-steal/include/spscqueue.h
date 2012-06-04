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

//Idea taken from http://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue

#ifndef _SPSCQUEUE_H_
#define _SPSCQUEUE_H_

#include "atomic.h"

namespace cpptask
{

template<class T>
class SPSCQueue
{
public:
    SPSCQueue()
    {
        Node* n = new Node;
        n->next = 0;
        head = tail = first = head_copy = n;
    }

    ~SPSCQueue()
    {
        Node* n = first;
        do
        {
            Node* next = n->next;
            delete n;
            n = next;
        }
        while (n);
    }

    void Push(T v)
    {
        Node* n = AllocNode();
        n->next = 0;
        n->value = v;
        StoreRelease(&tail->next, n);
        tail = n;
    }

    bool Pop(T& v)
    {
        if (LoadConsume(&head->next))
        {
            v = head->next->value;
            StoreRelease(&head, head->next);
            return true;
        }
        else
        {
            return false;
        }
    }
private:
    SPSCQueue(SPSCQueue const&);
    SPSCQueue& operator = (SPSCQueue const&);

    // internal node structure
    struct Node
    {
        Node* next;
        T value;
    };

    Node* AllocNode()
    {
        // first tries to allocate node from internal node cache,
        // if attempt fails, allocates node via ::operator new()

        if (first != head_copy)
        {
            Node* n = first;
            first = first->next;
            return n;
        }

        head_copy = LoadConsume(&head);

        if (first != head_copy)
        {
            Node* n = first;
            first = first->next;
            return n;
        }
        Node* n = new Node;
        return n;
    }

    // load with 'consume' (data-dependent) memory ordering
    template<class TT>
    static TT LoadConsume(TT const* addr)
    {
        // hardware fence is implicit on x86
        TT v = *const_cast<TT const volatile*>(addr);
        MemoryFence(); // compiler fence
        return v;
    }

    // store with 'release' memory ordering
    template<class TT>
    static void StoreRelease(TT* addr, TT v)
    {
        // hardware fence is implicit on x86
        MemoryFence(); // compiler fence
        *const_cast<TT volatile*>(addr) = v;
    }

private:
    // consumer part
    // accessed mainly by consumer, infrequently be producer
    Node* head; // head of the queue

    // delimiter between consumer part and producer part,
    // so that they situated on different cache lines
    char cache_line_pad [64];

    // producer part
    // accessed only by producer
    Node* tail; // tail of the queue
    Node* first; // last unused node (head of node cache)
    Node* head_copy; // helper (points somewhere between first and head)
    };
}

#endif

