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

#ifndef _TASKTHREADIMPL_H_
#define _TASKTHREADIMPL_H_

#include "task.h"
#include "taskmanager.h"

namespace cpptask
{

inline TaskThread::TaskThread(TaskThreadPool& threadPool, Event& newTaskEvent) 
    : newTaskEvent(newTaskEvent)
{
    manager.Reset(new TaskManager(threadPool, newTaskEvent));
}

inline TaskThread::~TaskThread()
{
    Stop();
    Wait();
}

inline void TaskThread::Run()
{
    manager->RegisterInTLS();

    while (!done.IsSet())
    {
        Task* task = manager->GetTask(this);
        if (task == 0)
        {
            std::vector<Event*> events(2);
            events[0] = &newTaskEvent;
            events[1] = &stopEvent;
            WaitForMultiple(events);
            task = manager->GetTask(this);
            newTaskEvent.Reset();
        }
        if (task != 0)
        {
            task->SetParentThread(this);
            task->Run();
            task->SignalDone();
        }
    }
}

inline void TaskThread::Stop()
{
    stopEvent.Signal();
    done.Set();
}

TaskManager* TaskThread::GetTaskManager()
{
    return manager.Get();
}

void TaskThread::DoWaitingTasks(Task* waitTask)
{
    //we are inside DoTask
    while (!waitTask->CheckFinished())
    {
        Task* task = manager->GetTask(this);
        if (task == 0)
        {
            std::vector<Event*> events(3);
            events[0] = &newTaskEvent;
            events[1] = &waitTask->waitEvent;
            events[2] = &stopEvent;
            if (WaitForMultiple(events) == 1)
            {
                return;
            }
            task = manager->GetTask(this);
        }
        if (task != 0)
        {
            task->SetParentThread(this);
            task->Run();
            task->SignalDone();
        }
    }
}

}
#endif
