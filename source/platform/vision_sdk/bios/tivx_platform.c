/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <xdc/std.h>
#include <osal/bsp_osal.h>
#include <src/rtos/links_common/system/system_priv_openvx.h>

uint64_t tivxPlatformGetTimeInUsecs(void)
{
    return BspOsal_getCurTimeInUsec();
}

void tivxPlatformPrintf(const char *format)
{
    printf(format);
}

void tivxPlatformActivate()
{
    System_openvxActivate();
}

void tivxPlatformDeactivate()
{
    System_openvxDeactivate();
}

char *tivxPlatformGetEnv(char *env_var)
{
    char *value=" ";
    
    if(strcmp(env_var, "VX_TEST_DATA_PATH")==0)
    {
        value="sd:test_data/";
    }
    return value;
}
