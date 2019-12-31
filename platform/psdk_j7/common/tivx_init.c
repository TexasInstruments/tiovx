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

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);
void tivxRegisterIVisionTargetKernels(void);
void tivxUnRegisterIVisionTargetKernels(void);
void tivxRegisterTutorialTargetKernels(void);
void tivxUnRegisterTutorialTargetKernels(void);
void tivxRegisterCaptureTargetArmKernels(void);
void tivxUnRegisterCaptureTargetArmKernels(void);
void tivxRegisterTestKernelsTargetC66Kernels(void);
void tivxUnRegisterTestKernelsTargetC66Kernels(void);

void tivxInit(void)
{
    tivx_set_debug_zone((int32_t)VX_ZONE_INIT);
    tivx_set_debug_zone((int32_t)VX_ZONE_ERROR);
    tivx_set_debug_zone((int32_t)VX_ZONE_WARNING);
    tivx_clr_debug_zone((int32_t)VX_ZONE_INFO);

    /* Initialize resource logging */
    tivxLogResourceInit();

    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

    /* Initialize Host */
#if defined (C66)
    tivxRegisterOpenVXCoreTargetKernels();
    #ifdef BUILD_TUTORIAL
    tivxRegisterTutorialTargetKernels();
    #endif
#endif

#ifdef BUILD_CONFORMANCE_TEST
#if defined (C66)
    tivxRegisterCaptureTargetArmKernels();
#endif

#if defined (C66)
    tivxRegisterTestKernelsTargetC66Kernels();
#endif
#endif

    /* Note: eventually register HWA kernels here (deferring for now) */

    tivxObjDescInit();

    tivxPlatformCreateTargets();

    VX_PRINT(VX_ZONE_INIT, "Initialization Done !!!\n");
}

void tivxDeInit(void)
{
    tivxPlatformDeleteTargets();

    /* DeInitialize Host */
#if defined (C66)
    tivxUnRegisterOpenVXCoreTargetKernels();
    #ifdef BUILD_TUTORIAL
    tivxUnRegisterTutorialTargetKernels();
    #endif
#endif

#ifdef BUILD_CONFORMANCE_TEST
#if defined (C66)
    tivxUnRegisterCaptureTargetArmKernels();
#endif

#if defined (C66)
    tivxUnRegisterTestKernelsTargetC66Kernels();
#endif
#endif

    /* Note: may need to run tivxHwaUnLoadKernels or something similar */

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();

    /* DeInitialize resource logging */
    tivxLogResourceDeInit();

    VX_PRINT(VX_ZONE_INIT, "De-Initialization Done !!!\n");
}
