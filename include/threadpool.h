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

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "refptr.h"
#include "semaphor.h"
#include "taskthread.h"
#include "taskmanager.h"
#include "tlskey.h"

namespace cpptask
{

class TaskThreadPool
{
public:
    TaskThreadPool(size_t threadsNum)
    {
        manager.Reset(new TaskManager(*this, newTaskEvent, 0));
        manager->RegisterInTLS();

        for (size_t i = 0; i < threadsNum; ++i)
        {
            TaskThreadPtr tptr(new TaskThread(*this, newTaskEvent));
            threads.push_back(tptr);
        }

        for (size_t i = 0; i < threadsNum; ++i)
        {
            threads[i]->Start();
        }
    }

    ~TaskThreadPool()
    {
        Threads::iterator i = threads.begin();
        Threads::iterator e = threads.end();
        for (;i != e; ++i)
        {
            (*i)->Stop();
            (*i)->Wait();
        }
    }

    size_t GetThreadsNum() const
    {
        return threads.size();
    }

    TaskThread* GetThread(size_t index)
    {
        if (index < threads.size())
        {
            return threads[index].Get();
        }
        return 0;
    }

    TaskManager* GetTaskManager()
    {
        return manager.Get();
    }

private:
    TaskThreadPool(const TaskThreadPool&);
    const TaskThreadPool& operator=(const TaskThreadPool&);
private:
    typedef RefPtr<TaskThread> TaskThreadPtr;
    typedef std::vector<TaskThreadPtr> Threads;
    Threads threads;
    Semaphore newTaskEvent;
    RefPtr<TaskManager> manager;
};

}
#endif
