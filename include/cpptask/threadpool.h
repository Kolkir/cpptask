/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2012, Kyrylo Kolodiazhnyi
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

#ifndef _CPP_TASK_THREADPOOL_H_
#define _CPP_TASK_THREADPOOL_H_

#include "eventmanager.h"
#include "taskthread.h"
#include "taskmanager.h"
#include "tlskey.h"

#include <memory>

namespace cpptask { namespace internal {


class TaskThreadPool
{
public:
    TaskThreadPool(size_t threadsNum)
    {
        taskManager.reset(new TaskManager(*this, eventManager, 0));
        taskManager->RegisterInTLS();

        for (size_t i = 0; i < threadsNum; ++i)
        {
            TaskThreadPtr tptr(new TaskThread(*this, eventManager));
            threads.push_back(tptr);
        }

        //Deffered start required to prevent race in already started threads during stealing new tasks
        for (size_t i = 0; i < threadsNum; ++i)
        {
            threads[i]->Start();
        }
    }

    ~TaskThreadPool()
    {
        for (size_t i = 0; i < threads.size(); ++i)
        {
            eventManager.notify(cpptask::internal::EventId::ThreadStopEvent); //notify multiple waits
        }
        for (auto& t : threads)
        {
            t->Wait();
        }
        taskManager->RemoveFromTLS();
    }
    
    TaskThreadPool(const TaskThreadPool&) = delete;

    const TaskThreadPool& operator=(const TaskThreadPool&) = delete;

    size_t GetThreadsNum() const
    {
        return threads.size();
    }

    TaskThread* GetThread(size_t index)
    {
        if (index < threads.size())
        {
            return threads[index].get();
        }
        return nullptr;
    }

    TaskManager& GetTaskManager()
    {
        return *taskManager;
    }

private:
    typedef std::shared_ptr<TaskThread> TaskThreadPtr;
    typedef std::vector<TaskThreadPtr> Threads;
    Threads threads;
    std::unique_ptr<TaskManager> taskManager;
    EventManager eventManager;
};

}}
#endif
