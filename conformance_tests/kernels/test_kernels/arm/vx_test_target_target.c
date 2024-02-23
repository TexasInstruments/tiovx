/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/tivx_test_kernels.h"
#include "VX/vx.h"
#include "tivx_capture_kernels.h"
#include "tivx_kernel_test_target.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include <TI/tivx_task.h>
#include <stdio.h>

/* #define FULL_CODE_COVERAGE */

static tivx_target_kernel vx_test_target_target_kernel = NULL;

static vx_status VX_CALLBACK tivxTestTargetProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestTargetCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestTargetDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxTestTargetControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

#if defined(FULL_CODE_COVERAGE)
#if defined(C7X_FAMILY)
#define TARGET_TEST_TASK_STACK_SIZE      64*1024U
#else
#define TARGET_TEST_TASK_STACK_SIZE      1024U
#endif
/* Note: there is probably a cleaner way of obtaining this value
 * with ifdefs, etc.
 * However, it should be greater than the below task values:
 *  - TIVX_TASK_MAX_OBJECTS for A72/A53
 *  - OSAL_FREERTOS_CONFIGNUM_TASK for PDK FreeRTOS
 *  - OSAL_SAFERTOS_CONFIGNUM_TASK for PDK SafeRTOS
 *  - APP_RTOS_MAX_TASK_COUNT for MCU+ */
#if defined(C7X_FAMILY)
#define TARGET_TEST_MAX_TASKS            128U
#else
#define TARGET_TEST_MAX_TASKS            1024U
#endif

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)

#if defined(C7X_FAMILY)
#define TARGET_TEST_TASK_STACK_ALIGNMENT 8*1024U
#else
#define TARGET_TEST_TASK_STACK_ALIGNMENT 1024U
#endif

static uint8_t tivxTestTargetTaskStack[TARGET_TEST_MAX_TASKS][TARGET_TEST_TASK_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(TARGET_TEST_TASK_STACK_ALIGNMENT)))
    ;

#endif

static void VX_CALLBACK tivxTestTask(void *app_var)
{
    do
    {
        tivxTaskWaitMsecs(1000);
    }
    while(1);
}

static vx_status tivxTestTargetTaskBoundary(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_task taskHandle[TARGET_TEST_MAX_TASKS];
    tivx_task_create_params_t taskParams;
    uint32_t i, j;

    tivxTaskSetDefaultCreateParams(&taskParams);
    taskParams.task_main = &tivxTestTask;
    taskParams.app_var = NULL;
    taskParams.stack_size = TARGET_TEST_TASK_STACK_SIZE;
    taskParams.core_affinity = TIVX_TASK_AFFINITY_ANY;
    taskParams.priority = 8;

    /* There isn't a clean way to query the total number of
     * tasks, so exiting once we hit the max */
    for (i = 0; i < TARGET_TEST_MAX_TASKS; i++)
    {
        snprintf(taskParams.task_name, TIVX_MAX_TASK_NAME, "TEST_%d", i);
        #if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
        taskParams.stack_ptr = tivxTestTargetTaskStack[i];
        #else
        taskParams.stack_ptr = NULL;
        #endif
        status = tivxTaskCreate(&taskHandle[i], &taskParams);

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    j = i;

    for (i = 0; i < j; i++)
    {
        status = tivxTaskDelete(&taskHandle[i]);
    }

    return status;
}
#endif /* FULL_CODE_COVERAGE */

static vx_status VX_CALLBACK tivxTestTargetProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_scalar_t *in_desc;
    tivx_obj_desc_scalar_t *out_desc;

    if ( (num_params != TIVX_KERNEL_TEST_TARGET_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_TEST_TARGET_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_TEST_TARGET_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_TEST_TARGET_INPUT_IDX];
        out_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_TEST_TARGET_OUTPUT_IDX];
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vx_uint8 in_value;

        in_value = in_desc->data.u08;

        out_desc->data.u08 = in_value;

    }

#if defined(FULL_CODE_COVERAGE)
    if((vx_status)VX_SUCCESS == status)
    {
        status = tivxTestTargetTaskBoundary();
    }
#endif /* FULL_CODE_COVERAGE */

    return status;
}

static vx_status VX_CALLBACK tivxTestTargetCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_TEST_TARGET_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_TEST_TARGET_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_TEST_TARGET_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxTestTargetDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxTestTargetControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelTestTarget(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) )
    {
        vx_test_target_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_TEST_TARGET_NAME,
                            target_name,
                            tivxTestTargetProcess,
                            tivxTestTargetCreate,
                            tivxTestTargetDelete,
                            tivxTestTargetControl,
                            NULL);
    }
    else if (self_cpu == TIVX_CPU_ID_MPU_0)
    {
        strncpy(target_name, TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
        vx_test_target_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_TEST_TARGET_NAME,
                            target_name,
                            tivxTestTargetProcess,
                            tivxTestTargetCreate,
                            tivxTestTargetDelete,
                            tivxTestTargetControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelTestTarget(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_test_target_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_test_target_target_kernel = NULL;
    }
}


