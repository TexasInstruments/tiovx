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
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

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
    }
}

vx_status tivxTaskCreate(tivx_task *task, tivx_task_create_params_t *params)
{
    vx_status status = VX_SUCCESS;
    Task_Handle tskHndl;
    Task_Params bios_task_prms;

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
        Task_Params_init(&bios_task_prms);

        bios_task_prms.stackSize = params->stack_size;
        bios_task_prms.stack = params->stack_ptr;
        bios_task_prms.priority = params->priority;
        bios_task_prms.arg0 = (UArg)(task);
        bios_task_prms.arg1 = (UArg)(task);

        tskHndl = (void*)Task_create(
                            (Task_FuncPtr)tivxTaskDefHandle,
                            &bios_task_prms, NULL);

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
        Task_delete((Task_Handle*)&task->tsk_handle);
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
    #if 1
    uint32_t ticks;

    /* Clock_tickPeriod is in units of usecs */
    ticks = (msec * 1000) / Clock_tickPeriod;

    TaskP_sleep(ticks);
    #else
    TaskP_yield(); /* using this until task sleep works fine in VLAB */
    #endif
}

