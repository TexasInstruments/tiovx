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
#include <utils/perf_stats/include/app_perf_stats.h>

static void tivxTaskDefHandle(void* arg0, void* arg1);

static void tivxTaskDefHandle(void* arg0, void* arg1)
{
    tivx_task *task;

    if (0U != (uintptr_t)arg0)
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
        strncpy(params->task_name, "TIVX", TIVX_MAX_TASK_NAME);
        params->task_name[TIVX_MAX_TASK_NAME-1U] = (char)0;
    }
}

vx_status tivxTaskCreate(tivx_task *task, const tivx_task_create_params_t *params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    TaskP_Handle tskHndl;
    TaskP_Params rtos_task_prms;

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
        TaskP_Params_init(&rtos_task_prms);

        rtos_task_prms.stacksize = params->stack_size;
        rtos_task_prms.stack     = params->stack_ptr;
        rtos_task_prms.priority  = (int8_t)params->priority;
        rtos_task_prms.arg0      = (void*)(task);
        rtos_task_prms.arg1      = (void*)(task);
        rtos_task_prms.name      = (const char*)&task->task_name[0];

        strncpy(task->task_name, params->task_name, TIVX_MAX_TASK_NAME);
        task->task_name[TIVX_MAX_TASK_NAME-1U] = (char)0;

        tskHndl = (void*)TaskP_create(
                            &tivxTaskDefHandle,
                            &rtos_task_prms);

        if (NULL == tskHndl)
        {
            VX_PRINT(VX_ZONE_ERROR, "Task handle could not be created\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            appPerfStatsRegisterTask(tskHndl, task->task_name);
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
        TaskP_delete((TaskP_Handle*)&task->tsk_handle);
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
    TaskP_sleepInMsecs(msec);
}

