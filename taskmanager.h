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
