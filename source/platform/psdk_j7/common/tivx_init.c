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
void tivxRegisterTestKernelsTargetDspKernels(void);
void tivxUnRegisterTestKernelsTargetDspKernels(void);
void tivxRegisterTestKernelsTargetArmKernels();
void tivxUnRegisterTestKernelsTargetArmKernels();

static void tivxInitLocal(void);
static void tivxDeInitLocal(void);

/* Counter for tracking the {init, de-init} calls. This is also used to
 * guarantee a single init/de-init operation.
 */
static uint32_t gInitCount = 0U;

#if defined(LINUX) || defined(QNX)
#include <pthread.h>

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void tivxInit(void)
{
    pthread_mutex_lock(&g_mutex);

    tivxInitLocal();

    pthread_mutex_unlock(&g_mutex);

}

void tivxDeInit(void)
{
    pthread_mutex_lock(&g_mutex);

    tivxDeInitLocal();

    pthread_mutex_unlock(&g_mutex);
}

#else
void tivxInit(void)
{
    tivxInitLocal();
}

void tivxDeInit(void)
{
    tivxDeInitLocal();
}
#endif // defined(LINUX) || defined(QNX)

static void tivxInitLocal(void)
{
    if (0U == gInitCount)
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
    #if defined (SOC_J721E)
    #if defined (C66)
        tivxRegisterOpenVXCoreTargetKernels();
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #endif

    #if defined (SOC_J721S2)
    #if defined (C71)
        tivxRegisterOpenVXCoreTargetKernels();
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #endif

    #ifdef BUILD_CONFORMANCE_TEST
    #if defined (R5F)
        tivxRegisterCaptureTargetArmKernels();
        tivxRegisterTestKernelsTargetArmKernels();
    #endif

    #if defined (SOC_J721E)
    #if defined (C66)
        tivxRegisterTestKernelsTargetDspKernels();
    #endif
    #endif

    #if defined (SOC_J721S2)
    #if defined (C71)
        tivxRegisterTestKernelsTargetDspKernels();
    #endif
    #endif
    #endif

        tivxObjDescInit();

        tivxPlatformCreateTargets();

        VX_PRINT(VX_ZONE_INIT, "Initialization Done !!!\n");
    }

    gInitCount++;
}

static void tivxDeInitLocal(void)
{
    if (0U != gInitCount)
    {
        gInitCount--;

        if (0U == gInitCount)
        {
            tivxPlatformDeleteTargets();

            /* DeInitialize Host */
        #if defined (SOC_J721E)
        #if defined (C66)
            tivxUnRegisterOpenVXCoreTargetKernels();
            #ifdef BUILD_TUTORIAL
            tivxUnRegisterTutorialTargetKernels();
            #endif
        #endif
        #endif

        #if defined (SOC_J721S2)
        #if defined (C71)
            tivxUnRegisterOpenVXCoreTargetKernels();
            #ifdef BUILD_TUTORIAL
            tivxUnRegisterTutorialTargetKernels();
            #endif
        #endif
        #endif

        #ifdef BUILD_CONFORMANCE_TEST
        #if defined (R5F)
            tivxUnRegisterCaptureTargetArmKernels();
            tivxUnRegisterTestKernelsTargetArmKernels();
        #endif

        #if defined (SOC_J721E)
        #if defined (C66)
            tivxUnRegisterTestKernelsTargetDspKernels();
        #endif
        #endif

        #if defined (SOC_J721S2)
        #if defined (C71)
            tivxUnRegisterTestKernelsTargetDspKernels();
        #endif
        #endif
        #endif

            /* DeInitialize Target */
            tivxTargetDeInit();

            /* DeInitialize platform */
            tivxPlatformDeInit();

            /* DeInitialize resource logging */
            tivxLogResourceDeInit();

            VX_PRINT(VX_ZONE_INIT, "De-Initialization Done !!!\n");
        }
    }
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
}
