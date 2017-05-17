/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <sys/time.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    uint64_t timeInUsecs = 0;
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0)
    {
        timeInUsecs = 0;
    }
    else
    {
        timeInUsecs = tv.tv_sec * 1000000ull + tv.tv_usec;
    }

    return timeInUsecs;
}

