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

#ifndef _TASKTHREADIMPL_H_
#define _TASKTHREADIMPL_H_

#include "task.h"
#include "taskmanager.h"
#include "waitoneof.h"

#include <assert.h>

namespace cpptask
{

inline TaskThread::TaskThread(TaskThreadPool& threadPool, semaphore& newTaskEvent)
    : newTaskEvent(newTaskEvent)
    , manager(new TaskManager(threadPool, newTaskEvent, this))
{
}

inline TaskThread::~TaskThread()
{
    Stop();
}

inline void TaskThread::Run()
{
    manager->RegisterInTLS();

    wait_one_of waits;
    waits.addEvent(newTaskEvent);
    waits.addEvent(stopEvent);

    bool done = false;
    while (!done)
    {
        Task* task = manager->GetTask();
        if (task == nullptr)
        {           
            int res = waits.wait();
            if (res == 0)
            {
                task = manager->GetTask();
            }
            else if (res == 1)
            {
                done = true;
                break;
            }
            else
            {
                assert(false);
            }
        }
        if (task != nullptr)
        {
            task->Run();
        }
    }
}

inline void TaskThread::Start()
{
    thread = std::move(std::thread(std::bind(&TaskThread::Run, this)));
}

inline void TaskThread::Stop()
{
    if (thread.joinable())
    {
        stopEvent.notify();
        thread.join();
    }
}

inline TaskManager& TaskThread::GetTaskManager()
{
    return *manager;
}

}
#endif
