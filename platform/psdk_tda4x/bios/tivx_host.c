/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_tda4x.h>

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);


void tivxHostInit(void)
{
    tivxObjectInit();
    tivxRegisterOpenVXCoreKernels();
    /* Note: eventually register HWA kernels here (deferring for now) */
    tivxPlatformSetHostTargetId(TIVX_TARGET_ID_A15_0);
    VX_PRINT(VX_ZONE_INIT, "Initialization Done for HOST !!!\n");
}

void tivxHostDeInit(void)
{
    VX_PRINT(VX_ZONE_INIT, "De-Initialization Done for HOST !!!\n");
    tivxObjectDeInit();
    tivxUnRegisterOpenVXCoreKernels();
    /* Note: eventually unregister HWA kernels here (deferring for now) */
}

