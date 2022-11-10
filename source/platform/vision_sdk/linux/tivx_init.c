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
        ownLogResourceInit();

        /* Initialize platform */
        ownPlatformInit();

        /* Initialize Target */
        ownTargetInit();

        /* Initialize Host */
        tivxHostInit();

        ownObjDescInit();

        ownPlatformCreateTargets();

        g_init_status = 1U;
    }
}

void tivxDeInit(void)
{
    if (1U == g_init_status)
    {
        ownPlatformDeleteTargets();

        /* DeInitialize Host */
        tivxHostDeInit();

        /* DeInitialize Target */
        ownTargetDeInit();

        /* DeInitialize platform */
        ownPlatformDeInit();

        /* DeInitialize resource logging */
        ownLogResourceDeInit();

        g_init_status = 0U;
    }
}
