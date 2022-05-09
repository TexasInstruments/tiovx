/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

#include <ti/osal/SemaphoreP.h>

vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;
    SemaphoreP_Handle handle;
    SemaphoreP_Params semParams;

    if (NULL != mutex)
    {
        /* Default parameter initialization */
        SemaphoreP_Params_init(&semParams);

        semParams.mode = SemaphoreP_Mode_BINARY;

        handle = SemaphoreP_create(1U, &semParams);

        if (NULL == handle)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Semaphore create returned NULL\n");
            VX_PRINT(VX_ZONE_ERROR, "  Check for memory leak, or may need to increase\n");
            #ifdef SYSBIOS
            VX_PRINT(VX_ZONE_ERROR, "  the value of OSAL_TIRTOS_MAX_SEMAPHOREP_PER_SOC\n");
            #elif FREERTOS
            VX_PRINT(VX_ZONE_ERROR, "  the value of OSAL_FREERTOS_MAX_SEMAPHOREP_PER_SOC\n");
            #elif SAFERTOS
            VX_PRINT(VX_ZONE_ERROR, "  the value of OSAL_SAFERTOS_MAX_SEMAPHOREP_PER_SOC\n");
            #endif
            VX_PRINT(VX_ZONE_ERROR, "  in pdk/packages/ti/osal/soc/<>/osal_soc.h\n");
        }
        else
        {
            *mutex = (tivx_mutex)handle;
            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;
    SemaphoreP_Status retVal;
    SemaphoreP_Handle handle;

    if ((NULL != mutex) && (*mutex != NULL))
    {
        handle = (tivx_mutex)*mutex;
        retVal = SemaphoreP_delete(handle);

        if (SemaphoreP_OK != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore delete returned an error\n");
        }
        else
        {
            *mutex = NULL;
            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;
    SemaphoreP_Status retVal;

    if (NULL != mutex)
    {
        retVal = SemaphoreP_pend((SemaphoreP_Handle)mutex,
            SemaphoreP_WAIT_FOREVER);

        if (SemaphoreP_OK != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore wait failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Mutex is NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;
    SemaphoreP_Status retVal;

    if (NULL != mutex)
    {
        retVal = SemaphoreP_post((SemaphoreP_Handle)mutex);
        if (SemaphoreP_OK != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore post returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}
