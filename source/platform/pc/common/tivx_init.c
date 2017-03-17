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

    #ifdef BUILD_IVISION_KERNELS
    /* trick target kernel used in EVE emulation mode to think
     * they are being invoked from a EVE
     */
    tivxSetSelfCpuId(TIVX_CPU_ID_EVE1);
    tivxRegisterIVisionTargetKernels();

    tivxSetSelfCpuId(TIVX_CPU_ID_EVE2);
    tivxRegisterIVisionTargetKernels();
    #endif

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
    #ifdef BUILD_IVISION_KERNELS
    tivxUnRegisterIVisionTargetKernels();
    #endif
    tivxHostDeInit();

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
