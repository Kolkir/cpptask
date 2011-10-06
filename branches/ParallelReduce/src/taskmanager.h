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
#include "event.h"
#include "atomic.h"
#include "mpscqueue.h"

namespace parallel
{

class TaskManager;
class ManagerThread: public Thread
{
public:
    ManagerThread():manager(0){}

    virtual void Run();

    void SetManager(TaskManager* manager){this->manager = manager;}
    
    void Finish()
    {
        taskProcessEvent.Signal();
        done.Set();
    }
    void NotifyScheduleTasks()
    {
        taskProcessEvent.Signal();
    }
private:
    Event taskProcessEvent;
    AtomicFlag done;
    TaskManager* manager;
};

class TaskManager
{
public:
    TaskManager(TaskThreadPool& threadPool)
        : threadPool(threadPool)
    {
        managerThread.SetManager(this);
        managerThread.Start();
    }

    ~TaskManager()
    {
        managerThread.Finish();
        managerThread.Wait();
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

    void StartTasks()
    {
        managerThread.NotifyScheduleTasks();
    }

    void ScheduleTasks()
    {
        MPSCNode* node = taskQueue.Pop();
        while(node != 0)
        {
            Task* task = static_cast<Task*>(node);

            TaskThread* thread = threadPool.GetEmptyThread();
            if (thread == 0)
            {
                thread = threadPool.GetEmptyThreadWait();
            }
            thread->SetTask(task);
 
            node = taskQueue.Pop();
        }
    }    

private:
    TaskThreadPool& threadPool;
    MPSCQueue taskQueue;
    ManagerThread managerThread;
};

void ManagerThread::Run()
{
    while(!done.IsSet())
    {
        taskProcessEvent.Wait();
        manager->ScheduleTasks();
        taskProcessEvent.Reset();
    }
}

}
#endif
