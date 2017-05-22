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
void tivxRegisterIVisionTargetKernels(void);
void tivxUnRegisterIVisionTargetKernels(void);
void tivxRegisterTutorialTargetKernels(void);
void tivxUnRegisterTutorialTargetKernels(void);

void tivxInit(void)
{
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

#if defined (EVE) && defined (BUILD_IVISION_KERNELS)
    tivxRegisterIVisionTargetKernels();
#endif

#if defined(HOST_CORE_IPU1_0)
#if defined (M4)
    tivxHostInit();
#endif
#endif

    tivxObjDescInit();

    tivxPlatformCreateTargets();
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

#if defined (EVE) && defined (BUILD_IVISION_KERNELS)
    tivxUnRegisterIVisionTargetKernels();
#endif

#if defined(HOST_CORE_IPU1_0)
#if defined (M4)
    tivxHostDeInit();
#endif
#endif

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
