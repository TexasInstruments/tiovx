/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

#include <xdc/std.h>
#include <osal/bsp_osal.h>


vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = (vx_status)VX_FAILURE;
    BspOsal_SemHandle handle;

    if (NULL != mutex)
    {
        handle = BspOsal_semCreate(1U, TRUE);

        if (NULL == handle)
        {
            status = (vx_status)VX_FAILURE;
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
    BspOsal_SemHandle handle;

    if ((NULL != mutex) && (*mutex != NULL))
    {
        handle = (tivx_mutex)*mutex;
        BspOsal_semDelete(&handle);
        *mutex = NULL;
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = (vx_status)VX_SUCCESS;
    Bool retVal;

    if (NULL != mutex)
    {
        retVal = BspOsal_semWait((BspOsal_SemHandle)mutex,
            BSP_OSAL_WAIT_FOREVER);

        if (FALSE == retVal)
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
    if (NULL != mutex)
    {
        BspOsal_semPost((BspOsal_SemHandle)mutex);
    }

    return ((vx_status)VX_SUCCESS);
}



