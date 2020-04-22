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

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);

static uint8_t g_init_status = 0U;

void tivxInit(void)
{
    if (0U == g_init_status)
    {
        tivx_set_debug_zone(VX_ZONE_INIT);
        tivx_set_debug_zone(VX_ZONE_ERROR);
        tivx_set_debug_zone(VX_ZONE_WARNING);

        /* Initialize resource logging */
        tivxLogResourceInit();

        /* Initialize platform */
        tivxPlatformInit();

        /* Initialize Target */
        tivxTargetInit();

        /* Initialize Host */
        tivxHostInit();

        tivxObjDescInit();

        tivxPlatformCreateTargets();

        g_init_status = 1U;
    }
}

void tivxDeInit(void)
{
    if (1U == g_init_status)
    {
        tivxPlatformDeleteTargets();

        /* DeInitialize Host */
        tivxHostDeInit();

        /* DeInitialize Target */
        tivxTargetDeInit();

        /* DeInitialize platform */
        tivxPlatformDeInit();

        /* DeInitialize resource logging */
        tivxLogResourceDeInit();

        g_init_status = 0U;
    }
}
