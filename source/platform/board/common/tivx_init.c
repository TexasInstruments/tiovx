/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */
#include <vx_internal.h>
#include <tivx_target_config.h>

void tivxRegisterOpenVXCoreTargetKernels(void);
void tivxUnRegisterOpenVXCoreTargetKernels(void);
void tivxRegisterTutorialTargetKernels(void);
void tivxUnRegisterTutorialTargetKernels(void);
void tivxRegisterCaptureTargetArmKernels(void);
void tivxUnRegisterCaptureTargetArmKernels(void);
void tivxRegisterTestKernelsTargetDspKernels(void);
void tivxUnRegisterTestKernelsTargetDspKernels(void);
void tivxRegisterTestKernelsTargetArmKernels(void);
void tivxUnRegisterTestKernelsTargetArmKernels(void);

static void tivxInitLocal(void);
static void tivxDeInitLocal(void);

/* Counter for tracking the {init, de-init} calls. This is also used to
 * guarantee a single init/de-init operation.
 */
static uint32_t gInitCount = 0U;

#if defined(LINUX) || defined(QNX)
#include <pthread.h>
#include <tivx_platform_posix.h>

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void tivxInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);

}

void tivxDeInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxDeInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);
}

#else
void tivxInit(void)
{
#if defined(REMOTE_COVERAGE)
/* LDRA dynamic analysis (code coverage capture) entry point.
 * The execution history capture will begin after this call for the
 * selected and ldra instrumented files.
 */
#include "code_coverage.h"
    appLogPrintf("################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
    ldra_initialize();
    appLogPrintf("################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
#endif /* #if defined REMOTE_COVERAGE */
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
        tivx_set_debug_zone((int32_t)VX_ZONE_ERROR);
        tivx_set_debug_zone((int32_t)VX_ZONE_WARNING);
        tivx_set_debug_zone((int32_t)VX_ZONE_INFO);

#if defined(LINUX) || defined(QNX)
        /* Initialize the POSIX objects */
        (void)ownPosixObjectInit();
#endif

        /* Initialize resource logging */
        ownLogResourceInit();

        /* Initialize platform */
        (void)ownPlatformInit();

        /* Initialize Target */
        ownTargetInit();

        /* Initialize Host */
    #if defined (SOC_J721E)
    #if defined (C66)
        #if defined(BUILD_CORE_KERNELS)
        tivxRegisterOpenVXCoreTargetKernels();
        #endif /* #if defined(BUILD_CORE_KERNELS) */
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #else
    #if defined (C7X_FAMILY)
        #if defined(BUILD_CORE_KERNELS)
        tivxRegisterOpenVXCoreTargetKernels();
        #endif /* #if defined(BUILD_CORE_KERNELS) */
        #ifdef BUILD_TUTORIAL
        tivxRegisterTutorialTargetKernels();
        #endif
    #endif
    #endif /* defined(SOC_J721E) */

    #ifdef BUILD_CONFORMANCE_TEST
    #if defined(BUILD_TEST_KERNELS)
    #if defined (R5F)
        tivxRegisterTestKernelsTargetArmKernels();
    #endif

    #if defined (C7X_FAMILY) || defined (C66) || defined (R5F)
    tivxRegisterCaptureTargetArmKernels();
    #endif

    #if defined (C7X_FAMILY) || defined (C66)
        tivxRegisterTestKernelsTargetDspKernels();
    #endif
    #endif /* #if defined(BUILD_TEST_KERNELS) */
    #endif  /* #ifdef BUILD_CONFORMANCE_TEST */

        ownObjDescInit();

        ownPlatformCreateTargets();

        VX_PRINT(VX_ZONE_INFO, "Initialization Done !!!\n");
        tivx_clr_debug_zone((int32_t)VX_ZONE_INFO);
    }

    gInitCount++;
}

static void tivxDeInitLocal(void)
{
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_INIT_UBR001
<justification end> */
    if (0U != gInitCount)
/* LDRA_JUSTIFY_END */
    {
        gInitCount--;

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_HOST_ONLY_INIT_UM001
<justification end> */
#endif
        if (0U == gInitCount)
        {
            ownPlatformDeleteTargets();

            /* DeInitialize Host */
        #if defined (SOC_J721E)
        #if defined (C66)
            #if defined(BUILD_CORE_KERNELS)
            tivxRegisterOpenVXCoreTargetKernels();
            #endif /* #if defined(BUILD_CORE_KERNELS) */
            #ifdef BUILD_TUTORIAL
            tivxRegisterTutorialTargetKernels();
            #endif
        #endif
        #else
        #if defined (C7X_FAMILY)
            #if defined(BUILD_CORE_KERNELS)
            tivxUnRegisterOpenVXCoreTargetKernels();
            #endif /* #if defined(BUILD_CORE_KERNELS) */
            #ifdef BUILD_TUTORIAL
            tivxUnRegisterTutorialTargetKernels();
            #endif
        #endif
        #endif /* defined(SOC_J721E) */

        #ifdef BUILD_CONFORMANCE_TEST
        #if defined(BUILD_TEST_KERNELS)
        #if defined (R5F)
            tivxUnRegisterTestKernelsTargetArmKernels();
        #endif

        #if defined (C7X_FAMILY) || defined (C66) || defined (R5F)
        tivxUnRegisterCaptureTargetArmKernels();
        #endif

        #if defined (C7X_FAMILY) || defined(C66)
            tivxUnRegisterTestKernelsTargetDspKernels();
        #endif
        #endif /* #if defined(BUILD_TEST_KERNELS) */
        #endif  /* #ifdef BUILD_CONFORMANCE_TEST */

            /* DeInitialize Target */
            ownTargetDeInit();

            /* DeInitialize platform */
            ownPlatformDeInit();

            /* DeInitialize resource logging */
            ownLogResourceDeInit();

#if defined(LINUX) || defined(QNX)
            /* DeInitialize the POSIX objects */
            (void)ownPosixObjectDeInit();
#endif

            VX_PRINT(VX_ZONE_INFO, "De-Initialization Done !!!\n");
        }
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
/* LDRA_JUSTIFY_END */
#endif
    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_INIT_UM001
<justification end> */
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
/* LDRA_JUSTIFY_END */
}
