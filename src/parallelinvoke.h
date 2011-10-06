#ifndef _PARALLEL_INVOKE_H_
#define _PARALLEL_INVOKE_H_

#include "taskmanager.h"

#include <algorithm>
#include <vector>

namespace parallel
{

template<class Functor>
class InvokeTask : public Task
{
public:
    InvokeTask(const Functor& functor)
        : functor(functor)
    {
    }
    ~InvokeTask()
    {
    }
    virtual void Execute()
    {
        functor();
    }

private:
    Functor functor;
};

template<class Functor1, class Functor2>
void ParallelInvoke(Functor1 func1, Functor2 func2, TaskManager& manager)
{           
    typedef InvokeTask<Functor1> TASK1;
    typedef std::shared_ptr<TASK1> TASKPtr1;

    TASKPtr1 task1(new TASK1(func1));
    manager.AddTask(task1.get());

    typedef InvokeTask<Functor2> TASK2;
    typedef std::shared_ptr<TASK2> TASKPtr2;

    TASKPtr2 task2(new TASK2(func2));
    manager.AddTask(task2.get());

    manager.StartTasks();

    task1->Wait();
    task2->Wait();
}

}

#endif
