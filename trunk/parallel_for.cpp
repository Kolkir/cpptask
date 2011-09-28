#include "parallel_for.h"
#include <process.h>

namespace parallel
{
/*****************************************************************************/
void ManagerThread::Run()
{
    while(done == 0 ||
          manager->HasQueuedTasks())
    {
        //here should be event listener
        manager->ScheduleTasks();
    }
}
/*****************************************************************************/
ThreadPool::ThreadPool(size_t threadsNum)
{
}
/*****************************************************************************/
size_t ThreadPool::GetThreadsNum() const
{
    return 0;
}
/*****************************************************************************/
TaskManager::TaskManager(ThreadPool& threadPool)
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
        thread->SetTask(task);
        threadPool.PushBackThread(thread);

        node = taskQueue.Pop();
    }
 }
 /*****************************************************************************/
}
