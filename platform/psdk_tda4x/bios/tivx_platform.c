/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <utils/console_io/include/app_log.h>
#include <utils/console_io/src/app_log_priv.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    return appLogGetTimeInUsec();
}

void tivxPlatformPrintf(const char *format)
{
    appLogPrintf(format);
}

void tivxPlatformActivate()
{

}

void tivxPlatformDeactivate()
{

}
