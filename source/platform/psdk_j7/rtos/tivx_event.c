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

vx_status tivxEventCreate(tivx_event *event)
{
    vx_status status = (vx_status)VX_FAILURE;
    SemaphoreP_Handle handle;
    SemaphoreP_Params semParams;

    if (NULL != event)
    {
        /* Default parameter initialization */
        SemaphoreP_Params_init(&semParams);

        semParams.mode = SemaphoreP_Mode_BINARY;

        handle = SemaphoreP_create(0U, &semParams);

        if (NULL == handle)
        {
            VX_PRINT(VX_ZONE_ERROR, "Semaphore creation failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            *event = (tivx_event)handle;
            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = (vx_status)VX_FAILURE;
    SemaphoreP_Handle handle;

    if ((NULL != event) && (*event != NULL))
    {
        handle = (SemaphoreP_Handle)*event;
        SemaphoreP_delete(handle);
        *event = NULL;
        status = (vx_status)VX_SUCCESS;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    if (NULL != event)
    {
        SemaphoreP_post((SemaphoreP_Handle)event);
    }

    return ((vx_status)VX_SUCCESS);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;
    SemaphoreP_Status retVal;
    uint32_t bsp_timeout;

    if (NULL != event)
    {
        if (TIVX_EVENT_TIMEOUT_WAIT_FOREVER == timeout)
        {
            bsp_timeout = SemaphoreP_WAIT_FOREVER;
        }
        else if (TIVX_EVENT_TIMEOUT_NO_WAIT == timeout)
        {
            bsp_timeout = SemaphoreP_NO_WAIT;
        }
        else
        {
            bsp_timeout = timeout;
        }

        retVal = SemaphoreP_pend((SemaphoreP_Handle)event, bsp_timeout);

        if (SemaphoreP_OK != retVal)
        {
            /* making info since on a valid timeout, it will continously print errors instead */
            VX_PRINT(VX_ZONE_INFO, "Semaphore wait returned an error\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Event was NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != event)
    {
        status = SemaphoreP_reset((SemaphoreP_Handle)event);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Event was NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}
