/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TASK_UBR001
<justification end> */
    if (0U != (uintptr_t)arg0)
    {
        task = (tivx_task *)arg0;

        task->task_func(task->app_var);
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_TASK_UM002
<justification end> */
}
/* LDRA_JUSTIFY_END */

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
            ownLogResourceAlloc("TIVX_TASK_MAX_OBJECTS", 1);
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
        ownLogResourceFree("TIVX_TASK_MAX_OBJECTS", 1);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_TASK_UM001
<justification end> */
        if (ret_status != (app_rtos_status_t)0)
        {
            VX_PRINT(VX_ZONE_ERROR,"Task_Delete: Task deletion failed\n");
            status = (vx_status)VX_FAILURE;
        }
/* LDRA_JUSTIFY_END */
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

