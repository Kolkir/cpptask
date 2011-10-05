#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <exception>
#include <string>

namespace parallel
{

class Exception: public std::exception
{
public:
    typedef std::exception BaseType;

    Exception(){}

    explicit Exception(const std::string& message)
    : BaseType(message.c_str())
    {
    }

    explicit Exception(const char *message)
    : BaseType(message)
    {
    }

    Exception* Clone() const
    {
        return new Exception(*this);
    }
};

}
#endif
