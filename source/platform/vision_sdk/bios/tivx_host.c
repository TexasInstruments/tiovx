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


void tivxHostInit(void)
{
    tivxObjectInit();
    tivxRegisterOpenVXCoreKernels();
    tivxPlatformSetHostTargetId(TIVX_TARGET_ID_IPU1_0);
    VX_PRINT(VX_ZONE_INIT, "Initialization Done for HOST !!!\n");
}

void tivxHostDeInit(void)
{
    VX_PRINT(VX_ZONE_INIT, "De-Initialization Done for HOST !!!\n");
    tivxObjectDeInit();
    tivxUnRegisterOpenVXCoreKernels();
}

