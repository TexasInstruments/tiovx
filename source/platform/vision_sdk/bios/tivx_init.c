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

void tivxInit(void)
{
    /* Initialize platform */
    tivxPlatformInit();

    /* Initialize Target */
    tivxTargetInit();

    /* Initialize Host */
#if defined (M4)
    tivxHostInit();
    tivxRegisterOpenVXCoreKernels();
#else
    tivxRegisterOpenVXCoreTargetKernels();
#endif
    tivxObjDescInit();

    tivxTargetConfig();
}

void tivxDeInit(void)
{
    /* DeInitialize Host */
#if defined(M4)
    tivxHostDeInit();
    tivxUnRegisterOpenVXCoreKernels();
#else
    tivxUnRegisterOpenVXCoreTargetKernels();
#endif

    /* DeInitialize Target */
    tivxTargetDeInit();

    /* DeInitialize platform */
    tivxPlatformDeInit();
}
