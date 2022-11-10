/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>
#include <tivx_platform_psdk.h>

void tivxRegisterOpenVXCoreKernels(void);
void tivxUnRegisterOpenVXCoreKernels(void);
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
    pthread_mutex_lock(&g_mutex);

    tivxHostInitLocal();

    pthread_mutex_unlock(&g_mutex);

}

void tivxHostDeInit(void)
{
    pthread_mutex_lock(&g_mutex);

    tivxHostDeInitLocal();

    pthread_mutex_unlock(&g_mutex);
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
        ownObjectInit();
        tivxRegisterOpenVXCoreKernels();

        #if defined(SOC_AM62A)
        if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_MCU1_0)
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MCU1_0);
        }
        #else
        if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_MCU2_0)
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MCU2_0);
        }
        #endif
        else
        if(tivxGetSelfCpuId()==(vx_enum)TIVX_CPU_ID_A72_0)
        {
            tivxPlatformSetHostTargetId(TIVX_TARGET_ID_A72_0);
        }
        else
        {
            /* do nothing */
        }

        VX_PRINT(VX_ZONE_INIT, "Initialization Done for HOST !!!\n");
    }

    gInitCount++;
}

static void tivxHostDeInitLocal(void)
{
    if (0U != gInitCount)
    {
        gInitCount--;

        if (0U == gInitCount)
        {
            VX_PRINT(VX_ZONE_INIT, "De-Initialization Done for HOST !!!\n");
            ownObjectDeInit();
            tivxUnRegisterOpenVXCoreKernels();
        }
    }
    else
    {
        /* ERROR. */
        VX_PRINT(VX_ZONE_ERROR, "De-Initialization Error !!!\n");
    }
}

