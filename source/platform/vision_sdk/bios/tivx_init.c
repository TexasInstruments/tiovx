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
void tivxRegisterCaptureTargetArmKernels(void);
void tivxUnRegisterCaptureTargetArmKernels(void);
void tivxRegisterIVisionTargetKernels(void);
void tivxUnRegisterIVisionTargetKernels(void);
void tivxRegisterTutorialTargetKernels(void);
void tivxUnRegisterTutorialTargetKernels(void);
void tivxRegisterTestKernelsTargetC66Kernels(void);
void tivxUnRegisterTestKernelsTargetC66Kernels(void);
void tivxRegisterTIDLTargetKernels(void);
void tivxUnRegisterTIDLTargetKernels(void);

void tivxInit(void)
{
    tivx_set_debug_zone(VX_ZONE_INIT);
    tivx_set_debug_zone(VX_ZONE_ERROR);
    tivx_set_debug_zone(VX_ZONE_WARNING);
    tivx_clr_debug_zone(VX_ZONE_INFO);

    /* Initialize resource logging */
    tivxLogResourceInit();

    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

#if defined (C66)
    tivxRegisterOpenVXCoreTargetKernels();
    #ifdef BUILD_TUTORIAL
    tivxRegisterTutorialTargetKernels();
    #endif
#endif

#if (defined (C66) || defined (EVE))
    tivxRegisterTIDLTargetKernels();
#endif

#if defined (EVE) && defined (BUILD_IVISION_KERNELS)
    tivxRegisterIVisionTargetKernels();
#endif

#ifdef BUILD_CONFORMANCE_TEST
#if defined (C66)
    tivxRegisterCaptureTargetArmKernels();
#endif

#if defined (C66)
    tivxRegisterTestKernelsTargetC66Kernels();
#endif
#endif

    tivxObjDescInit();

    tivxPlatformCreateTargets();

    VX_PRINT(VX_ZONE_INIT, "Initialization Done !!!\n");
}

void tivxDeInit(void)
{
    tivxPlatformDeleteTargets();

#ifdef BUILD_CONFORMANCE_TEST
#if defined (C66)
    tivxUnRegisterCaptureTargetArmKernels();
#endif

#if defined (C66)
    tivxUnRegisterTestKernelsTargetC66Kernels();
#endif
#endif

    /* DeInitialize Host */
#if defined (C66)
    tivxUnRegisterOpenVXCoreTargetKernels();
    #ifdef BUILD_TUTORIAL
    tivxUnRegisterTutorialTargetKernels();
    #endif
#endif

#if (defined (C66) || defined (EVE))
    tivxUnRegisterTIDLTargetKernels();
#endif

#if defined (EVE) && defined (BUILD_IVISION_KERNELS)
    tivxUnRegisterIVisionTargetKernels();
#endif

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();

    /* DeInitialize resource logging */
    tivxLogResourceDeInit();

    VX_PRINT(VX_ZONE_INIT, "De-Initialization Done !!!\n");
}
