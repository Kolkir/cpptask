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

#ifndef _PARALLEL_INVOKE_H_
#define _PARALLEL_INVOKE_H_

namespace cpptask
{

template<class Functor>
class InvokeTask : public Task
{
public:
    InvokeTask(const Functor& functor)
        : functor(functor)
    {
    }
    ~InvokeTask()
    {
    }
    virtual void Execute()
    {
        functor();
    }

private:
    Functor functor;
};

template<class Functor1, class Functor2>
inline void ParallelInvoke(Functor1 func1, Functor2 func2)
{
    typedef InvokeTask<Functor1> TASK1;
    typedef RefPtr<TASK1> TASKPtr1;

    TaskManager* manager = TaskManager::GetCurrent();

    TASKPtr1 task1(new(manager->GetCacheLineSize()) TASK1(func1));
    manager->AddTask(task1.Get());

    typedef InvokeTask<Functor2> TASK2;
    typedef RefPtr<TASK2> TASKPtr2;

    TASKPtr2 task2(new(manager->GetCacheLineSize()) TASK2(func2));
    manager->AddTask(task2.Get());

    manager->WaitTask(task1.Get());
    manager->WaitTask(task2.Get());

    if (task1->GetLastException() != 0)
    {
        task1->GetLastException()->Throw();
    }
    if (task2->GetLastException() != 0)
    {
        task2->GetLastException()->Throw();
    }
}

}

#endif
