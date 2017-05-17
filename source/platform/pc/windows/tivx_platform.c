/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <windows.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    uint64_t timeInUsecs = 0;
    LARGE_INTEGER value, hz, tmp;
    BOOL is_ok;

    is_ok = QueryPerformanceCounter(&value);
    if(is_ok)
    {
        is_ok = QueryPerformanceFrequency(&hz);

        tmp.QuadPart = (value.QuadPart*1000000ULL/hz.QuadPart);

        timeInUsecs = tmp.QuadPart;
    }

    return timeInUsecs;
}

