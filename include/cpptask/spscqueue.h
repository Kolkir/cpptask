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

#include <cassert>
#include <cstdalign>
#include <atomic>

namespace cpptask
{

template<class T>
class SPSCQueue
{
public:

    SPSCQueue()
    {
        Node* n = new Node;
        n->next.store(nullptr, std::memory_order_relaxed);
        head.store(n, std::memory_order_relaxed);
        tail = n;
        cache_start = cache_end = n;
    }

    ~SPSCQueue()
    {
        Node* n = cache_start;
        do
        {
            Node* next = n->next.load(std::memory_order_relaxed);
            delete n;
            n = next;
        }
        while (n != nullptr);
    }

    void Push(T v)
    {
        Node* n = AllocNode();
        n->next = nullptr;
        n->value = v;
        tail->next.store(n, std::memory_order_release);
        tail = n;
    }

    bool Pop(T& v)
    {
        auto h = head.load(std::memory_order_acquire);
        auto h_next = h->next.load(std::memory_order_acquire);
        if (h_next != nullptr)
        {
            v = h_next->value;
            assert(v != 0);
            head.store(h_next, std::memory_order_release);
            return true;
        }
        else
        {
            return false;
        }
    }

    SPSCQueue(SPSCQueue const&) = delete;
    SPSCQueue& operator = (SPSCQueue const&) = delete;
private:
    // internal node structure
    struct Node
    {
        std::atomic<Node*> next;
        T value;
    };

    Node* AllocNode()
    {
        // cache_start tries to allocate node from internal node cache,
        // if attempt fails, allocates node via ::operator new()

        if (cache_start != cache_end)
        {
            Node* n = cache_start;
            cache_start = cache_start->next.load(std::memory_order_acquire);
            return n;
        }

        cache_end = head.load(std::memory_order_acquire);

        if (cache_start != cache_end)
        {
            Node* n = cache_start;
            cache_start = cache_start->next.load(std::memory_order_acquire);
            return n;
        }
        Node* n = new Node;
        return n;
    }

//private:
    public:
    // consumer part
    // accessed mainly by consumer, infrequently be producer
    alignas(64) std::atomic<Node*> head; // head of the queue

    // producer part
    // accessed only by producer
    alignas(64) Node* tail; // tail of the queue
    Node* cache_start; // first unused node (head of node cache)
    Node* cache_end; // helper (points somewhere between cache_start and head)
};

}

#endif

