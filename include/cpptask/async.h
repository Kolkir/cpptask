/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2015, Kyrylo Kolodiazhnyi
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

#ifndef _CPP_TASK_ASYNC_H_
#define _CPP_TASK_ASYNC_H_

#include <future>
#include <chrono>

namespace cpptask
{
    template <class R>
    class future 
    {
    public:
        future() noexcept
        {}

        //contructor for internal use only
        future(std::unique_ptr<internal::Task> task, std::future<R> f, bool deffered) noexcept
            : task(std::move(task))
            , realFuture(std::move(f))
            , deffered(deffered)
        {}

        future(future&& f)
            : task(std::move(f.task))
            , realFuture(std::move(f.realFuture))
            , deffered(f.deffered)
        {}

        future(const future& rhs) = delete;
        
        future& operator=(const future& rhs) = delete;

        ~future() 
        {}
        
        future& operator=(future&& f) noexcept
        {
            task = std::move(f.task);
            realFuture = std::move(f.realFuture);
            deffered = f.deffered;
        }

        R get()
        {
            auto& manager = internal::TaskManager::GetCurrent();
            if (deffered)
            {
                manager.DoTask(*task);
                return realFuture.get();
            }
            else
            {
                if (!task->IsFinished())
                {                    
                    manager.WaitTask(*task);
                }
                return realFuture.get();
            }
        }

        bool valid() const noexcept
        {
            return realFuture.valid();
        }

        void wait() const
        {
            if (!task->IsFinished())
            {
                if (!deffered)
                {
                    auto& manager = internal::TaskManager::GetCurrent();
                    manager.WaitTask(*task);
                }
            }
        }

        template <class Rep, class Period>
        std::future_status wait_for(const std::chrono::duration<Rep, Period>& rel_time) const
        {
            return realFuture.wait_for(rel_time);
        }

        template <class Clock, class Duration>
        std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const
        {
            return realFuture.wait_until(abs_time);
        }

    private:
        std::unique_ptr<internal::Task> task;
        std::future<R> realFuture;
        bool deffered;
    };

    namespace internal
    {
        template<class Functor>
        class AsyncTask : public Task
        {
        public:
            AsyncTask(Functor&& functor)
                : functor(std::forward<Functor>(functor))
            {
            }
            ~AsyncTask()
            {
            }
            virtual void Execute()
            {
                functor();
            }
        private:
            Functor functor;
        };
    }
    
    template< class Function, class... Args >
    future<typename std::result_of<Function(Args...)>::type> async(std::launch policy, Function&& f, Args&&... args)
    {
        std::packaged_task <typename std::result_of<Function(Args...)>::type()> job(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
        auto res = job.get_future();
        std::unique_ptr<internal::Task> task(new internal::AsyncTask<decltype(job)>(std::move(job)));

        if (policy == std::launch::deferred)
        {
            return future<typename std::result_of<Function(Args...)>::type>(std::move(task), std::move(res), true);
        }
        else if (policy == std::launch::async)
        {
            auto& manager = internal::TaskManager::GetCurrent();
            manager.AddTask(*task);
            return future<typename std::result_of<Function(Args...)>::type>(std::move(task), std::move(res), false);
        }
        else
        {
            throw exception("Wrong launch policy for async");
        }
    }
}

#endif
