/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2012, Kirill Kolodyazhnyi
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

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "event.h"
#include "mutex.h"
#include "multwait.h"
#include <semaphore.h>
#include <stdexcept>

namespace cpptask
{

class Semaphore : public MultWaitBase<Event, Mutex>
{
public:
    Semaphore()
    {
        int err = sem_init(&psem, 0, 0);
        if (err != 0)
        {
            throw std::runtime_error("Can't create a semaphore");
        }
    }
    ~Semaphore()
    {
        sem_destroy(&psem);
    }

    void Wait()
    {
        int err = sem_wait(&psem);
        if (err != 0)
        {
            //log error
        }
    }

    void Signal()
    {
        Lock();
        int err = sem_post(&psem);
        if (err != 0)
        {
            //log error
        }
        MultSignal();
        UnLock();
    }

    virtual bool MultCheck()
    {
        int err = sem_trywait(&psem);
        if (err == 0)
        {
            return true;
        }
        return false;
    }

private:
    Semaphore(const Semaphore&);
    const Semaphore& operator=(const Semaphore&);

private:
    sem_t psem;
};

}

#endif

