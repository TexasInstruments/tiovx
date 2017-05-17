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



