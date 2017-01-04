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



void tivxTaskDefHandle(UInt32 arg0, UInt32 arg1)
{
    tivx_task *task;

    if (0 != arg0)
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
            params->priority,
            params->stack_ptr,
            params->stack_size,
            task,
            params->core_affinity);

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

    if ((NULL != task) && (NULL != task->tsk_handle))
    {
        BspOsal_taskDelete(task->tsk_handle);
    }
    else
    {
        status = VX_FAILURE;
    }

    return (status);
}

void tivxTaskWaitMsecs(uint32_t msec)
{
    BspOsal_sleep(msec);
}

