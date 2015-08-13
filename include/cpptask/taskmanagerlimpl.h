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

#ifndef _TASKMANAGERIMPL_H_
#define _TASKMANAGERIMPL_H_

#include "taskmanager.h"
#include "threadpool.h"
#include "waitoneof.h"

#include <assert.h>

namespace cpptask
{

inline TaskManager::TaskManager(TaskThreadPool& threadPool, semaphore& newTaskEvent, TaskThread* parentThread)
    : parentThread(parentThread)
    , threadPool(threadPool)
    , newTaskEvent(newTaskEvent)
{
}

inline TaskManager::~TaskManager()
{
}

inline size_t TaskManager::GetThreadsNum() const
{
    return threadPool.GetThreadsNum();
}

inline void TaskManager::AddTask(Task& task)
{
    taskQueue.Enqueue(&task);
    newTaskEvent.notify();
}

inline Task* TaskManager::GetOwnTask()
{
    Task* res = nullptr;
    std::unique_lock<mutex> lock(getGuard, std::try_to_lock);
    if (lock)
    {
        if (!taskQueue.Dequeue(res))
        {
            res = nullptr;
        }
    }
    return res;
}

inline Task* TaskManager::GetTask()
{
    Task* res = GetOwnTask();
    if (res == nullptr)
    {
        res = threadPool.GetTaskManager().GetOwnTask();
        if (res == nullptr)
        {
            for (size_t i = 0; i < threadPool.GetThreadsNum(); ++i)
            {
                TaskThread* thread = threadPool.GetThread(i);
                assert(thread != nullptr);
                if (thread != parentThread)
                {
                    res = thread->GetTaskManager().GetOwnTask();
                    if (res != nullptr)
                    {
                        break;
                    }
                }
            }
        }
    }
    return res;
}

inline TaskManager& TaskManager::GetCurrent()
{
    void* pvalue = GetManagerKey().GetValue();
    if (pvalue != nullptr)
    {
        return *reinterpret_cast<TaskManager*>(pvalue);
    }
    else
    {
        throw Exception("Can't acquire current task manager");
    }
}

inline void TaskManager::RegisterInTLS()
{
    GetManagerKey().SetValue(this);
}

inline void TaskManager::WaitTask(Task& waitTask)
{
    while (!waitTask.CheckFinished())
    {
        Task* task = GetTask();
        if (task == nullptr)
        {
            wait_array events = { &newTaskEvent, waitTask.GetWaitEvent() };
            int res = wait_one_of(events);
            if (res == 0)
            {
                task = GetTask();
            }
        }
        if (task != nullptr)
        {
            task->Run();
            task->SignalDone();
        }
    }
}

}
#endif
