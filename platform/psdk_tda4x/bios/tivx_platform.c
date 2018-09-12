/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    /* TODO: Find this API */
    //return BspOsal_getCurTimeInUsec();
    return 0;
}

void tivxPlatformPrintf(const char *format)
{
    printf(format);
}

void tivxPlatformActivate()
{

}

void tivxPlatformDeactivate()
{

}
