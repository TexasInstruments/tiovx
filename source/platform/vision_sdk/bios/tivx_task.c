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

static void tivxTaskDefHandle(UInt32 arg0, UInt32 arg1);

static void tivxTaskDefHandle(UInt32 arg0, UInt32 arg1)
{
    tivx_task *task;

    if (0U != arg0)
    {
        task = (tivx_task *)arg0;

        task->task_func(task->app_var);
    }
}

void tivxTaskSetDefaultCreateParams(tivx_task_create_params_t *params)
{
    if (NULL != params)
    {
        memset(params, 0, sizeof(tivx_task_create_params_t));

        params->core_affinity = TIVX_TASK_AFFINITY_ANY;
        params->priority = TIVX_TASK_PRI_LOWEST;
    }
}

vx_status tivxTaskCreate(tivx_task *task, const tivx_task_create_params_t *params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    BspOsal_TaskHandle tskHndl;

    if ((NULL != task) && (NULL != params))
    {
        task->stack_ptr = params->stack_ptr;
        task->stack_size = params->stack_size;
        task->core_affinity = params->core_affinity;
        task->priority = params->priority;
        task->task_func = params->task_main;
        task->app_var = params->app_var;

        tskHndl = (void *)BspOsal_taskCreate(
            tivxTaskDefHandle,
            (char*)params->task_name,
            params->priority,
            params->stack_ptr,
            params->stack_size,
            task,
            params->core_affinity);

        if (NULL == tskHndl)
        {
            VX_PRINT(VX_ZONE_ERROR, "Task handle could not be created\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            task->tsk_handle = (void *)tskHndl;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Task or params are NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

vx_status tivxTaskDelete(tivx_task *task)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != task) && (NULL != task->tsk_handle))
    {
        BspOsal_taskDelete(task->tsk_handle);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Task or task handle are NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

void tivxTaskWaitMsecs(uint32_t msec)
{
    BspOsal_sleep(msec);
}

