/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk_j7.h>

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);
void tivxPlatformResetObjDescTableInfo(void);

void tivxHostInit(void)
{
    /* Dont init Obj Desc table here, since its done during system time by RTOS core
     * This API will be called by each TIOVX linux process.
     * So if obj desc are init every time a TIOVX process is created
     * then some other TIOVX process running in background would lose its state and things will go wrong
     */
    /* tivxPlatformResetObjDescTableInfo(); */
    tivxObjectInit();
    tivxRegisterOpenVXCoreKernels();

    if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_IPU1_0)
    {
        tivxPlatformSetHostTargetId(TIVX_TARGET_ID_IPU1_0);
    }
    else
    if(tivxGetSelfCpuId()==TIVX_CPU_ID_A72_0)
    {
        tivxPlatformSetHostTargetId(TIVX_TARGET_ID_A72_0);
    }
    else
    {
        /* do nothing */
    }

    /* Note: eventually register HWA kernels here (deferring for now) */

    VX_PRINT(VX_ZONE_INIT, "Initialization Done for HOST !!!\n");
}

void tivxHostDeInit(void)
{
    VX_PRINT(VX_ZONE_INIT, "De-Initialization Done for HOST !!!\n");
    tivxObjectDeInit();
    tivxUnRegisterOpenVXCoreKernels();
    /* Note: eventually unregister HWA kernels here (deferring for now) */
}

