/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_vision_sdk.h>

void tivxTargetConfig(void)
{
}

void tivxInit(void)
{
    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

    /* Initialize Host */
    if (TIVX_CPU_ID_IPU1_0 == tivxIpcGetSelfCpuId())
    {
        tivxHostInit();
    }

    tivxTargetConfig();
}

void tivxDeInit(void)
{
    /* DeInitialize Host */
    if (TIVX_CPU_ID_IPU1_0 == tivxIpcGetSelfCpuId())
    {
        tivxHostDeInit();
    }

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
