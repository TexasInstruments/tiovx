/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
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
