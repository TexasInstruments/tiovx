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
void tivxRegisterTestKernelsTargetArmKernels();
void tivxUnRegisterTestKernelsTargetArmKernels();

static uint8_t g_init_status = 0U;

void tivxInit(void)
{
    if (0U == g_init_status)
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
        tivxRegisterTestKernelsTargetArmKernels();
    #endif

    #if defined (C66)
        tivxRegisterTestKernelsTargetC66Kernels();
    #endif
    #endif

        tivxObjDescInit();

        tivxPlatformCreateTargets();

        g_init_status = 1U;

        VX_PRINT(VX_ZONE_INIT, "Initialization Done !!!\n");
    }
}

void tivxDeInit(void)
{
    if (1U == g_init_status)
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
        tivxUnRegisterTestKernelsTargetArmKernels();
    #endif

    #if defined (C66)
        tivxUnRegisterTestKernelsTargetC66Kernels();
    #endif
    #endif

        /* DeInitialize Target */
        tivxTargetDeInit();

        /* DeInitialize platform */
        tivxPlatformDeInit();

        /* DeInitialize resource logging */
        tivxLogResourceDeInit();

        g_init_status = 0U;

        VX_PRINT(VX_ZONE_INIT, "De-Initialization Done !!!\n");
    }
}
