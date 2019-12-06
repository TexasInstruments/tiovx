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

void tivxPlatformActivate(void)
{

}

void tivxPlatformDeactivate(void)
{

}

char *tivxPlatformGetEnv(const char *env_var)
{
    const char *value=" ";
    
    if(strcmp(env_var, "VX_TEST_DATA_PATH")==0)
    {
        value="/test_data/";
    }
    return ((char *)value);
}
