/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>
#include <windows.h>

vx_status tivxEventCreate(tivx_event *event)
{
    vx_status status = VX_SUCCESS;
    HANDLE hSem;

    hSem = CreateSemaphore(
            NULL,
            0, /* init value */
            1, /* max value */
            NULL);

    *event = hSem;

    if(hSem==NULL)
    {
        status = VX_ERROR_NO_RESOURCES;
    }

    return (status);
}

vx_status tivxEventDelete(tivx_event *event)
{
    vx_status status = VX_SUCCESS;

    if(event && *event)
    {
        CloseHandle((HANDLE)*event);
        *event = NULL;
    }

    return (status);
}

vx_status tivxEventPost(tivx_event event)
{
    vx_status status = VX_SUCCESS;
    HANDLE hSem = (HANDLE)event;

    if(hSem)
    {
        if( !ReleaseSemaphore(hSem, 1, NULL) )
        {
            status = VX_FAILURE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

vx_status tivxEventWait(tivx_event event, uint32_t timeout)
{
    vx_status status = VX_SUCCESS;
    HANDLE hSem = (HANDLE)event;

    if(hSem)
    {
        DWORD dwWaitResult;
        DWORD dwWait;

        if(timeout==TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
        {
            dwWait = INFINITE;
        }
        else
        {
            dwWait = timeout;
        }

        dwWaitResult = WaitForSingleObject(
                        hSem,
                        dwWait);

        if(dwWaitResult != WAIT_OBJECT_0)
        {
            status = VX_FAILURE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    vx_status status;

    status = tivxEventWait(event, 0U);

    return status;
}
