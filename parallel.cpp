#include "parallel.h"

namespace parallel
{
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
}
/*****************************************************************************/
void TaskManager::StartTask(Task* task)
{
    if (task != 0)
    {
        task->Execute();
    }
}
/*****************************************************************************/
void Task::Wait() const
{
}
/*****************************************************************************/
}
