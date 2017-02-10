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

#include <xdc/std.h>
#include <osal/bsp_osal.h>


vx_status tivxEventCreate(tivx_event *event)
{
    vx_status status = VX_FAILURE;
    BspOsal_SemHandle handle;

    if (NULL != event)
    {
        handle = BspOsal_semCreate(0, FALSE);

        if (NULL == handle)
        {
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
            status = VX_FAILURE;
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxEventClear(tivx_event event)
{
    /* Should call Semaphore_reset (0) */
    return VX_SUCCESS;
}
