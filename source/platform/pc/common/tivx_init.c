/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_pc.h>

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);

void tivxRegisterIVisionTargetKernels(void);
void tivxUnRegisterIVisionTargetKernels(void);

void tivxInit(void)
{
    tivx_set_debug_zone(VX_ZONE_INIT);
    tivx_set_debug_zone(VX_ZONE_ERROR);
    tivx_set_debug_zone(VX_ZONE_WARNING);

    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

    /* Initialize Host */

    /* trick target kernel used in DSP emulation mode to think
     * they are being invoked from a DSP
     */
    tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);
    tivxRegisterOpenVXCoreTargetKernels();

    tivxSetSelfCpuId(TIVX_CPU_ID_DSP2);
    tivxRegisterOpenVXCoreTargetKernels();

    /* let rest of system think it is running on DSP1 */
    tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);

    tivxHostInit();

    tivxObjDescInit();

    tivxPlatformCreateTargets();
}

void tivxDeInit(void)
{
    tivxPlatformDeleteTargets();

    /* DeInitialize Host */
    tivxUnRegisterOpenVXCoreTargetKernels();

    tivxHostDeInit();

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
