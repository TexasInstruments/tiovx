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


vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = VX_FAILURE;
    BspOsal_SemHandle handle;

    if (NULL != mutex)
    {
        handle = BspOsal_semCreate(1U, FALSE);

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
    BspOsal_SemHandle handle;

    if ((NULL != mutex) && (*mutex != NULL))
    {
        handle = (tivx_mutex)*mutex;
        BspOsal_semDelete(&handle);
        *mutex = NULL;
        status = VX_SUCCESS;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = VX_SUCCESS;
    Bool retVal;

    if (NULL != mutex)
    {
        retVal = BspOsal_semWait((BspOsal_SemHandle)mutex,
            BSP_OSAL_WAIT_FOREVER);

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

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    if (NULL != mutex)
    {
        BspOsal_semPost((BspOsal_SemHandle)mutex);
    }

    return (VX_SUCCESS);
}



