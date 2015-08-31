/*
* https://github.com/Kolkir/cpptask/
* Copyright (c) 2012, Kyrylo Kolodiazhnyi
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

#ifndef _CPP_TASK_TLSKEY_H_
#define _CPP_TASK_TLSKEY_H_

#include "../exception.h"

#include <Windows.h>
#include <assert.h>
#include "winerrmsg.h"

namespace cpptask { namespace internal {

class TLSKey
{
public:
    TLSKey()
    {
        tlsIndex = ::TlsAlloc();
        if (tlsIndex == TLS_OUT_OF_INDEXES)
        {
            throw exception("Can't create a TLS key - " + GetLastWinErrMsg());
        }
    }

    ~TLSKey()
    {
        if (!::TlsFree(tlsIndex))
        {
            assert(false);
        }
    }

    TLSKey(const TLSKey&) = delete;

    const TLSKey& operator=(const TLSKey&) = delete;

    void* GetValue() const
    {
        void* rez = TlsGetValue(tlsIndex);
        if (rez == nullptr && ::GetLastError() != ERROR_SUCCESS)
        {
            throw exception("Can't get a TLS value - " + GetLastWinErrMsg());
        }
        return rez;
    }

    void SetValue(void* value)
    {
        if (!::TlsSetValue(tlsIndex, value))
        {
            throw exception("Can't set a TLS value - " + GetLastWinErrMsg());
        }
    }

private:
    DWORD tlsIndex;
};

}}

#endif
