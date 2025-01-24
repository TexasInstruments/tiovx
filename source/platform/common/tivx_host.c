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

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);
void tivxRegisterOpenVXExtKernels(void);
void tivxUnRegisterOpenVXExtKernels(void);
void tivxPlatformResetObjDescTableInfo(void);

static uint8_t gInitCount = 0U;

static void tivxHostInitLocal(void);
static void tivxHostDeInitLocal(void);

#if defined(LINUX) || defined(QNX)
#include <pthread.h>

/* Mutex for controlling access to Init/De-Init. */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void tivxHostInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxHostInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);

}

void tivxHostDeInit(void)
{
    (void)pthread_mutex_lock(&g_mutex);

    tivxHostDeInitLocal();

    (void)pthread_mutex_unlock(&g_mutex);
}

#else
void tivxHostInit(void)
{
    tivxHostInitLocal();
}

void tivxHostDeInit(void)
{
    tivxHostDeInitLocal();
}
#endif // defined(LINUX) || defined(QNX)

static void tivxHostInitLocal(void)
{
    if (0U == gInitCount)
    {
        /* Dont init Obj Desc table here, since its done during system time by RTOS core
         * This API will be called by each TIOVX linux process.
         * So if obj desc are init every time a TIOVX process is created
         * then some other TIOVX process running in background would lose its state and things will go wrong
         */
        /* tivxPlatformResetObjDescTableInfo(); */

        /* This function registers functions that only makes sense on the host.  This is done to optimize
           memory footprint so the linker on the non-host cores can optimize out symbols needed on the host.
         */
        ownRegisterFunctionsForHost();
        (void)ownObjectInit();
        tivxRegisterOpenVXCoreKernels();
        tivxRegisterOpenVXExtKernels();

        if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_MPU_0) /* TIOVX-1946- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_HOST_UBR001 */
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MPU_0);
        }
        #if defined(SOC_AM62A)
        else if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_MCU1_0)
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MCU1_0);
        }
        #else
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1732- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_HOST_UM001 */
        else if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_MCU2_0)
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MCU2_0);
        }
#endif
        #endif
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1732- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_HOST_UM002 */
        else
        {
            /* do nothing */
        }
#endif

        VX_PRINT(VX_ZONE_INFO, "Initialization Done for HOST !!!\n");
    }

    gInitCount++;
}

static void tivxHostDeInitLocal(void)
{
    if (0U != gInitCount) /* TIOVX-1946- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_HOST_UBR002 */
    {
        gInitCount--;

        if (0U == gInitCount)
        {
            VX_PRINT(VX_ZONE_INFO, "De-Initialization Done for HOST !!!\n");
            (void)ownObjectDeInit();
            tivxUnRegisterOpenVXExtKernels();
            tivxUnRegisterOpenVXCoreKernels();
        }
    }
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1732- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_HOST_UM003 */
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
#endif
}
