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

vx_status tivxMutexCreate(tivx_mutex *mutex)
{
    vx_status status = VX_SUCCESS;
    HANDLE hMutex;

    hMutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    *mutex = (tivx_mutex)hMutex;

    if(hMutex==NULL)
    {
        status = VX_ERROR_NO_RESOURCES;
    }

    return (status);
}

vx_status tivxMutexDelete(tivx_mutex *mutex)
{
    vx_status status = VX_SUCCESS;

    if(mutex && *mutex)
    {
        CloseHandle((HANDLE)*mutex);
        *mutex = NULL;
    }

    return (status);
}

vx_status tivxMutexLock(tivx_mutex mutex)
{
    vx_status status = VX_SUCCESS;
    HANDLE hMutex = (HANDLE)mutex;

    if(hMutex)
    {
        DWORD dwWaitResult;

        dwWaitResult = WaitForSingleObject(
                        hMutex,
                        INFINITE);

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

vx_status tivxMutexUnlock(tivx_mutex mutex)
{
    vx_status status = VX_SUCCESS;
    HANDLE hMutex = (HANDLE)mutex;

    if(hMutex)
    {
        if( !ReleaseMutex(hMutex) )
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



