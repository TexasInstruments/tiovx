/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

#include <utils/rtos/include/app_rtos.h>

static void tivxTaskDefHandle(void* arg0, void* arg1);
void init_params(tivx_task_create_params_t ** const params);

void init_params(tivx_task_create_params_t ** const params)
{
    (*params)->stack_ptr = NULL;
    (*params)->stack_size = (uint32_t)0;
    (*params)->core_affinity = (uint32_t)0;
    (*params)->priority = (uint32_t)0;
    (*params)->task_main = NULL;
    (*params)->app_var = NULL;
    (void)memset(&(*params)->task_name, 0, sizeof((*params)->task_name));

    return;
}

static void tivxTaskDefHandle(void* arg0, void* arg1)
{
    tivx_task *task;

    if (0U != (uintptr_t)arg0) /* TIOVX-1885- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_TASK_UBR001 */
    {
        task = (tivx_task *)arg0;

        task->task_func(task->app_var);
    }
/* TIOVX-1761- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_RTOS_TASK_UM002 */
}

void tivxTaskSetDefaultCreateParams(tivx_task_create_params_t *params)
{
    if (NULL != params)
    {
        init_params(&params);

        params->core_affinity = TIVX_TASK_AFFINITY_ANY;
        params->priority = TIVX_TASK_PRI_LOWEST;
        (void)strncpy(params->task_name, "TIVX", TIVX_MAX_TASK_NAME);
        params->task_name[TIVX_MAX_TASK_NAME-1U] = (char)0;
    }
}

vx_status tivxTaskCreate(tivx_task *task, const tivx_task_create_params_t *params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_task_handle_t tskHndl;
    app_rtos_task_params_t rtos_task_prms;

    if ((NULL != task) && (NULL != params) && (NULL != params->task_main))
    {
        /* Filling tivx_task structure */
        task->stack_ptr            = params->stack_ptr;
        task->stack_size           = params->stack_size;
        task->core_affinity        = params->core_affinity;
        task->priority             = params->priority;
        task->task_func            = params->task_main;
        task->app_var              = params->app_var;

        /* Filling app_rtos_task_params_t structure */
        appRtosTaskParamsInit(&rtos_task_prms);

        rtos_task_prms.stacksize = params->stack_size;
        rtos_task_prms.stack     = params->stack_ptr;
        rtos_task_prms.priority  = (uint8_t)params->priority;
        rtos_task_prms.arg0      = (void*)(task);
        rtos_task_prms.arg1      = (void*)(task);
        rtos_task_prms.name      = (const char*)&task->task_name[0];
        rtos_task_prms.taskfxn   = &tivxTaskDefHandle;


        (void)strncpy(task->task_name, params->task_name, TIVX_MAX_TASK_NAME);
        task->task_name[TIVX_MAX_TASK_NAME-1U] = (char)0;

        tskHndl = (void*)appRtosTaskCreate(&rtos_task_prms);

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
    app_rtos_status_t ret_status = APP_RTOS_STATUS_SUCCESS;

    if ((NULL != task) && (NULL != task->tsk_handle))
    {
        ret_status = appRtosTaskDelete((app_rtos_task_handle_t*)&task->tsk_handle);
#ifdef LDRA_UNTESTABLE_CODE
/* TIOVX-1761- LDRA Uncovered Id: TIOVX_CODE_COVERAGE_RTOS_TASK_UM001 */
        if (ret_status != (app_rtos_status_t)0)
        {
            VX_PRINT(VX_ZONE_ERROR,"Task_Delete: Task deletion failed\n");
            status = (vx_status)VX_FAILURE;
        }
#endif
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
    appRtosTaskSleepInMsecs(msec);
}

