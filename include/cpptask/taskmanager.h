/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2011, Kyrylo Kolodiazhnyi
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

#ifndef _CPP_TASK_TASKMANAGER_H_
#define _CPP_TASK_TASKMANAGER_H_

#include "spscqueue.h"
#include "tlskey.h"
#include "eventmanager.h"

namespace cpptask { namespace internal {

class Task;
class TaskThreadPool;
class Thread;
class TaskThread;

inline TLSKey& GetManagerKey()
{
    static TLSKey managerKey;
    return managerKey;
}

class TaskManager
{
public:
    TaskManager(TaskThreadPool& threadPool, EventManager& eventManager, TaskThread* parentThread);

    ~TaskManager();

    TaskManager(const TaskManager&) = delete;

    TaskManager& operator=(const TaskManager&) = delete;

    size_t GetThreadsNum() const;

    void AddTask(Task& task);

    Task* GetOwnTask();

    Task* GetTask();

    static TaskManager& GetCurrent();

    EventManager& GetEventManager();

    void RegisterInTLS();

    void RemoveFromTLS();

    void WaitTask(Task& waitTask);

    void DoTask(Task& task);
    void DoOneTask();

private:
    TaskThread* parentThread;
    TaskThreadPool& threadPool;
    SPSCQueue<Task*> taskQueue;
    std::mutex getGuard;
    EventManager& eventManager;
};

}}
#endif
