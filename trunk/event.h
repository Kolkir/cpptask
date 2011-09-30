#ifndef _EVENT_H_
#define _EVENT_H_

#include <windows.h>

namespace parallel
{

class Event
{
public:
    Event()
    {    
        // Manual reset since multiple threads can wait on this
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(hEvent == NULL)
        {
            throw std::runtime_error("Can't create event.");
        }
    }
    ~Event()
    {  
        ::CloseHandle(hEvent);
    }
    void Wait()
    {
        ::WaitForSingleObject(hEvent, INFINITE);
    }
    void Signal()
    {
        ::SetEvent(hEvent);    
    }
    void Reset()
    {
        ::ResetEvent(hEvent);
    }
private:
    Event(const Event&);
    const Event& operator=(const Event&);
private:
   HANDLE hEvent;
};

}

#endif