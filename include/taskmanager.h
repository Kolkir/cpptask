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

#include "spscqueue.h"
#include "tlskey.h"
#include "mutex.h"
#include "semaphor.h"
#include "alignedalloc.h"

namespace cpptask
{

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
    TaskManager(TaskThreadPool& threadPool, Semaphore& newTaskEvent, TaskThread* parentThread);

    ~TaskManager();

    size_t GetThreadsNum() const;

    void AddTask(Task* task);

    Task* GetOwnTask();

    Task* GetTask();

    size_t GetCacheLineSize() const;

    static TaskManager* GetCurrent();

    void RegisterInTLS();

    void WaitTask(Task* waitTask);

private:
    TaskThread* parentThread;
    TaskThreadPool& threadPool;
    SPSCQueue<Task*> taskQueue;
    size_t cacheLineSize;
    Mutex getGuard;
    Semaphore& newTaskEvent;
};


}
#endif
