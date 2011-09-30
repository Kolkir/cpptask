#include "parallel_for.h"
#include <process.h>

namespace parallel
{
/*****************************************************************************/
void ManagerThread::Run()
{
    while(!done.IsSet())
    {
        taskProcessEvent.Wait();
        manager->ScheduleTasks();
        taskProcessEvent.Reset();
    }
}
/*****************************************************************************/
TaskManager::TaskManager(TaskThreadPool& threadPool)
    : threadPool(threadPool)
{
    managerThread.SetManager(this);
    managerThread.Start();
}
/*****************************************************************************/
TaskManager::~TaskManager()
{
    managerThread.Finish();
    managerThread.Wait();
}
/*****************************************************************************/
void TaskManager::StartTask(Task* task)
{    
    if (task != 0)
    {
       taskQueue.Push(task);
       managerThread.NotifyScheduleTasks();
    }
}
/*****************************************************************************/
 void TaskManager::ScheduleTasks()
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
/*****************************************************************************/
TaskThreadPool::TaskThreadPool(size_t threadsNum)
{
    for (size_t i = 0; i < threadsNum; ++i)
    {
        TaskThreadPtr tptr(new TaskThread(&emptyThreadEvent));
        threads.push_back(tptr);
        tptr->Start();
    }
}
/*****************************************************************************/
size_t TaskThreadPool::GetThreadsNum() const
{
    return threads.size();
}
/*****************************************************************************/
TaskThread* TaskThreadPool::GetEmptyThread()
{
    Threads::iterator i =
    std::find_if(threads.begin(), threads.end(),
        [&](TaskThreadPtr& thread)->bool
    {
        if (!thread->HasTask())
        {
            return true;
        }
        return false;
    });
    if (i != threads.end())
    {
        return i->get();
    }
    return 0;
}
/*****************************************************************************/
TaskThread* TaskThreadPool::GetEmptyThreadWait()
{
    emptyThreadEvent.Wait();
    TaskThread* rez = GetEmptyThread();
    emptyThreadEvent.Reset();
    return rez;
}
/*****************************************************************************/
}
