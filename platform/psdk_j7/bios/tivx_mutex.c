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
    vx_status status = VX_FAILURE;
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
            status = VX_FAILURE;
        }
        else
        {
            *mutex = (tivx_mutex)handle;
            status = VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = VX_FAILURE;
    SemaphoreP_Handle handle;

    if ((NULL != mutex) && (*mutex != NULL))
    {
        handle = (tivx_mutex)*mutex;
        SemaphoreP_delete(handle);
        *mutex = NULL;
        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = VX_SUCCESS;
    uint32_t retVal;

    if (NULL != mutex)
    {
        retVal = SemaphoreP_pend((SemaphoreP_Handle)mutex,
            SemaphoreP_WAIT_FOREVER);

        if (SemaphoreP_OK != retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxMutexLock: Semaphore wait failed\n");
            status = VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxMutexLock: Mutex is NULL\n");
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    if (NULL != mutex)
    {
        SemaphoreP_post((SemaphoreP_Handle)mutex);
    }

    return (VX_SUCCESS);
}



