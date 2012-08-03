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

#ifndef _TLSKEY_H_
#define _TLSKEY_H_

#include <Windows.h>
#include <stdexcept>
#include <assert.h>

namespace cpptask
{

class TLSKey
{
public:
    TLSKey()
    {
        tlsIndex = ::TlsAlloc();
        if (tlsIndex == TLS_OUT_OF_INDEXES)
        {
            throw std::runtime_error("Can't create a TLS key");
        }
    }

    ~TLSKey()
    {
        if (!::TlsFree(tlsIndex))
        {
            assert(false);
        }
    }
   
    void* GetValue() const
    {
        return TlsGetValue(tlsIndex);
    }

    void SetValue(void* value)
    {
        if (!::TlsSetValue(tlsIndex, value))
        {
            assert(false);
        }
    }

private:
    TLSKey(const TLSKey&);
    const TLSKey& operator=(const TLSKey&);
private:
    DWORD tlsIndex;
};

}

#endif
