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

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);

static uint8_t g_init_status = 0U;

void tivxHostInit(void)
{
    if (0U == g_init_status)
    {
        ownObjectInit();
        tivxRegisterOpenVXCoreKernels();
        tivxPlatformSetHostTargetId(TIVX_TARGET_ID_IPU1_0);
        g_init_status = 1U;
        VX_PRINT(VX_ZONE_INIT, "Initialization Done for HOST !!!\n");
    }
}

void tivxHostDeInit(void)
{
    if (1U == g_init_status)
    {
        VX_PRINT(VX_ZONE_INIT, "De-Initialization Done for HOST !!!\n");
        ownObjectDeInit();
        g_init_status = 0U;
        tivxUnRegisterOpenVXCoreKernels();
    }
}

