/*
* http://code.google.com/p/cpptask/
* Copyright (c) 2011, Kirill Kolodyazhnyi
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

#ifndef _ALIGNED_ALLOC_H_
#define _ALIGNED_ALLOC_H_

#include <stdlib.h>
#include <malloc.h>
#include <windows.h>

namespace cpptask
{
#ifdef __GNUC__
#ifndef HAVE_PROCESSOR_CACHE_TYPE
typedef enum _PROCESSOR_CACHE_TYPE
{
    CacheUnified,
    CacheInstruction,
    CacheData,
    CacheTrace
} PROCESSOR_CACHE_TYPE;
#endif

#ifndef HAVE_CACHE_DESCRIPTOR
typedef struct _CACHE_DESCRIPTOR
{
    BYTE Level;
    BYTE Associativity;
    WORD LineSize;
    DWORD Size;
    PROCESSOR_CACHE_TYPE Type;
} CACHE_DESCRIPTOR, *PCACHE_DESCRIPTOR;
#endif

#ifndef HAVE_LOGICAL_PROCESSOR_RELATIONSHIP
typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP
{
    RelationProcessorCore,
    RelationNumaNode,
    RelationCache,
    RelationProcessorPackage,
    RelationGroup,
    RelationAll = 0xffff,
} LOGICAL_PROCESSOR_RELATIONSHIP;
#endif

#ifndef HAVE_SYSTEM_LOGICAL_PROCESSOR_INFORMATION
typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION
{
    ULONG_PTR ProcessorMask;
    LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
    _ANONYMOUS_UNION
    union
    {
        struct
        {
            BYTE flags;
        } ProcessorCore;
        struct
        {
            DWORD NodeNumber;
        } NumaNode;
        CACHE_DESCRIPTOR Cache;
        ULONGLONG Reserved[2];
    } DUMMYUNIONNAME;
} SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;
#endif
#endif

typedef BOOL (WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
    PDWORD);

inline size_t GetCacheLineSize()
{
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

    LPFN_GLPI glpi = (LPFN_GLPI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32")),
                            "GetLogicalProcessorInformation");
    if (0 == glpi)
    {
        throw std::logic_error("Can't get address of GetLogicalProcessorInformation function");
    }

    glpi(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    glpi(&buffer[0], &buffer_size);

    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
    {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
        {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }

    free(buffer);

    return line_size;
}

inline void* AlignedAlloc(size_t size, size_t align_size)
{
#ifdef __GNUC__
    return __mingw_aligned_malloc(size, align_size);
#else
    return _aligned_malloc(size, align_size);
#endif
}

inline void AlignedFree(void* ptr)
{
#ifdef __GNUC__
    return __mingw_aligned_free(ptr);
#else
    return _aligned_free(ptr);
#endif
}

}

#endif
