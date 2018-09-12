/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <ti/osal/TaskP.h>

static void tivxTaskDefHandle(uint32_t arg0, uint32_t arg1);

static void tivxTaskDefHandle(uint32_t arg0, uint32_t arg1)
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

vx_status tivxTaskCreate(tivx_task *task, tivx_task_create_params_t *params)
{
    vx_status status = VX_SUCCESS;
    TaskP_Handle tskHndl;
    TaskP_Params taskParams;

    if ((NULL != task) && (NULL != params))
    {
        /* Filling tivx_task structure */
        task->stack_ptr            = params->stack_ptr;
        task->stack_size           = params->stack_size;
        task->core_affinity        = params->core_affinity;
        task->priority             = params->priority;
        task->task_func            = params->task_main;
        task->app_var              = params->app_var;

        /* Filling TaskP_Params structure as defined in TaskP.h */
        taskParams.priority       = params->priority;
        taskParams.stacksize      = params->stack_size;
        taskParams.arg0           = task;
        taskParams.arg1           = task;
        taskParams.name           = (uint8_t *)params->task_name;

        tskHndl = (void *)TaskP_create(
            tivxTaskDefHandle,
            &taskParams);

        if (NULL == tskHndl)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTaskCreate: Task handle could not be created\n");
            status = VX_FAILURE;
        }
        else
        {
            task->tsk_handle = (void *)tskHndl;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxTaskCreate: Task or params are NULL\n");
        status = VX_FAILURE;
    }

    return (status);
}

vx_status tivxTaskDelete(tivx_task *task)
{
    vx_status status = VX_SUCCESS;

    if ((NULL != task) && (NULL != task->tsk_handle))
    {
        TaskP_delete(task->tsk_handle);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxTaskDelete: Task or task handle are NULL\n");
        status = VX_FAILURE;
    }

    return (status);
}

void tivxTaskWaitMsecs(uint32_t msec)
{
    TaskP_sleep(msec);
}

