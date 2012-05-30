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

#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_

#include "thread.h"
#include "task.h"
#include "spscqueue.h"
#include "tlskey.h"
#include "mutex.h"

namespace cpptask
{

class TaskManager
{
public:
    TaskManager(TaskThreadPool& threadPool)
        : threadPool(threadPool)
    {
        cacheLineSize = cpptask::GetCacheLineSize();
    }

    ~TaskManager()
    {
    }

    size_t GetThreadsNum() const
    {
        return threadPool.GetThreadsNum();
    }

    void AddTask(Task* task)
    {
        if (task != 0)
        {
           taskQueue.Push(task);
        }
    }

    Task* GetOwnTask()
    {
        Task* res = 0;
        if (getGuard.TryLock())
        {
            taskQueue.Pop(res);
            getGuard.UnLock();
        }
        return res;
    }

    Task* GetTask(const Thread* excludeThread)
    {
        Task* res = GetOwnTask();
        if (res == 0)
        {
            for (int i = 0; i < threadPool.GetThreadsNum(); ++i)
            {
                Thread* thread = threadPool.GetThread(i);
                if (thread != excludeThread)
                {
                    res = .GetTaskManager().GetOwnTask();
                    if (res != 0)
                    {
                        break;
                    }
                }
            }
        }
        return res;
    }

    size_t GetCacheLineSize() const
    {
        return cacheLineSize;
    }

    static TaskManager* GetCurrent()
    {
        static TLSKey tlsKey;
        void* pvalue = tlsKey.GetValue();
        if (pvalue != 0)
        {
            return reinterpret_cast<TaskManager*>(pvalue);
        }
        return 0;
    }

private:
    TaskThreadPool& threadPool;
    SPSCQueue<Task*> taskQueue;
    size_t cacheLineSize;
    Mutex getGuard;
};


}
#endif
