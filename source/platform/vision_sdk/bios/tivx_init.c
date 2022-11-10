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
void tivxRegisterTestKernelsTargetDspKernels(void);
void tivxUnRegisterTestKernelsTargetDspKernels(void);
void tivxRegisterTIDLTargetKernels(void);
void tivxUnRegisterTIDLTargetKernels(void);

static uint8_t g_init_status = 0U;

void tivxInit(void)
{
    if (0U == g_init_status)
    {
        tivx_set_debug_zone(VX_ZONE_INIT);
        tivx_set_debug_zone(VX_ZONE_ERROR);
        tivx_set_debug_zone(VX_ZONE_WARNING);
        tivx_clr_debug_zone(VX_ZONE_INFO);

        /* Initialize resource logging */
        ownLogResourceInit();

        /* Initialize platform */
        ownPlatformInit();

        /* Initialize Target */
        ownTargetInit();

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
        tivxRegisterTestKernelsTargetDspKernels();
    #endif
    #endif

        ownObjDescInit();

        ownPlatformCreateTargets();

        g_init_status = 1U;

        VX_PRINT(VX_ZONE_INIT, "Initialization Done !!!\n");
    }
}

void tivxDeInit(void)
{
    if (1U == g_init_status)
    {
        ownPlatformDeleteTargets();

    #ifdef BUILD_CONFORMANCE_TEST
    #if defined (C66)
        tivxUnRegisterCaptureTargetArmKernels();
    #endif

    #if defined (C66)
        tivxUnRegisterTestKernelsTargetDspKernels();
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
        ownTargetDeInit();

        /* DeInitialize platform */
        ownPlatformDeInit();

        /* DeInitialize resource logging */
        ownLogResourceDeInit();

        g_init_status = 0U;

        VX_PRINT(VX_ZONE_INIT, "De-Initialization Done !!!\n");
    }
}
