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


vx_status tivxEventCreate(tivx_event *event)
{
    vx_status status = VX_FAILURE;
    BspOsal_SemHandle handle;

    if (NULL != event)
    {
        handle = BspOsal_semCreate(0U, TRUE);

        if (NULL == handle)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventCreate: Semaphore creation failed\n");
            status = VX_FAILURE;
        }
        else
        {
            *event = (tivx_event)handle;
            status = VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = VX_FAILURE;
    BspOsal_SemHandle handle;

    if ((NULL != event) && (*event != NULL))
    {
        handle = (BspOsal_SemHandle)*event;
        BspOsal_semDelete(&handle);
        *event = NULL;
        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    if (NULL != event)
    {
        BspOsal_semPost((BspOsal_SemHandle)event);
    }

    return (VX_SUCCESS);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = VX_SUCCESS;
    Bool retVal;
    uint32_t bsp_timeout;

    if (NULL != event)
    {
        if (TIVX_EVENT_TIMEOUT_WAIT_FOREVER == timeout)
        {
            bsp_timeout = BSP_OSAL_WAIT_FOREVER;
        }
        else
        {
            bsp_timeout = BSP_OSAL_NO_WAIT;
        }

        retVal = BspOsal_semWait((BspOsal_SemHandle)event, bsp_timeout);

        if (FALSE == retVal)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxEventWait: Semaphore wait returned an error\n");
            status = VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxEventWait: Event was NULL\n");
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    /* Should call Semaphore_reset (0) */
    return VX_SUCCESS;
}
