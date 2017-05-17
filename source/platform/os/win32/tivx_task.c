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



void tivxTaskSetDefaultCreateParams(tivx_task_create_params_t *params)
{
    if (NULL != params)
    {
        memset(params, 0, sizeof(tivx_task_create_params_t));

        params->core_affinity = TIVX_TASK_AFFINITY_ANY;
        params->priority = TIVX_TASK_PRI_LOWEST;
    }
}

DWORD WINAPI tivxTaskDefHandle(LPVOID lpParameter)
{
    tivx_task *task = (tivx_task *)lpParameter;

    if( task && task->task_func)
    {
        task->task_func(task->app_var);
    }
    return 0U;
}

vx_status tivxTaskCreate(tivx_task *task, tivx_task_create_params_t *params)
{
    vx_status status = VX_SUCCESS;
    void * tskHndl;

    if ((NULL != task) && (NULL != params))
    {
        task->stack_ptr = params->stack_ptr;
        task->stack_size = params->stack_size;
        task->core_affinity = params->core_affinity;
        task->priority = params->priority;
        task->task_func = params->task_main;
        task->app_var = params->app_var;

        tskHndl = (void *)CreateThread(
            NULL,
            params->stack_size,
            tivxTaskDefHandle,
            task,
            0,
            NULL);

        if (NULL == tskHndl)
        {
            status = VX_FAILURE;
        }
        else
        {
            task->tsk_handle = (void *)tskHndl;
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxTaskDelete(tivx_task *task)
{
    vx_status status = VX_SUCCESS;

    if(task && task->tsk_handle)
    {
        HANDLE  hThread[1];

        hThread[0] = task->tsk_handle;

        WaitForMultipleObjects(1, hThread, TRUE, INFINITE);
        CloseHandle(task->tsk_handle);
        task->tsk_handle = NULL;
    }

    return (status);
}

void tivxTaskWaitMsecs(uint32_t msec)
{
    Sleep(msec);
}

