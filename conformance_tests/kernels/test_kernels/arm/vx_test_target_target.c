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
#include <TI/tivx_queue.h>
#include <tivx_obj_desc_priv.h>
#include <tivx_target.h>
#include <stdio.h>
#include <tivx_target_kernel_priv.h>
#include <tivx_obj_desc_queue.h>
#include <tivx_target_kernel_instance.h>
#include <tivx_target.h>
#include <utils/mem/include/app_mem.h>

#ifndef PC
#include <tivx_platform_psdk.h>
#include <utils/ipc/include/app_ipc.h>
#endif

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
#include <utils/rtos/include/app_rtos.h>
#if defined(MCU_PLUS_SDK)
#include <app_rtos_mcu_plus_priv.h>
#else
#include <osal_soc.h>
#endif
#endif
#if defined(A72) || defined(A53) || defined(PC)
#include <tivx_platform_posix.h>
#endif

/* #define FULL_CODE_COVERAGE */
/* Maximum length of testcase function name */
#define MAX_LENGTH 64
#define INVALID_ARG -1
#define IPC_MP_INVALID_ID            (0xFFFFFFFFU)
#define APP_IPC_HW_SPIN_LOCK_MAX        (256u)
#define INVALID_TARGET "invalid"
#define  TASK_IS_NOT_TERMINATED 0u

#define INVALID_MEM_REGION    APP_MEM_HEAP_MAX + 1

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
static tivx_target_kernel_instance test_kernel = NULL;
static tivx_obj_desc_t *test_obj_desc = NULL;
tivx_obj_desc_t *g_obj_desc[TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST] = {NULL};

typedef struct {
    vx_status (*funcPtr)(uint8_t);
    char funcName[MAX_LENGTH];
    vx_status status;
} FuncInfo;

FuncInfo arrOfFuncs[];

vx_status create_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
vx_status process_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

vx_status create_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

vx_status process_function(tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static void VX_CALLBACK tivxTestTask(void *app_var)
{
    do
    {
        tivxTaskWaitMsecs(1000);
    }
    while(1);
}

static vx_status tivxTestTargetTaskBoundary(uint8_t id)
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
        #if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
        /* Test case to cover "appRtosTaskIsTerminated" API */
        if (TASK_IS_NOT_TERMINATED != appRtosTaskIsTerminated((app_rtos_task_handle_t *)taskHandle[i].tsk_handle))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'tskHndl' \n");
        }
        #endif
        status = tivxTaskDelete(&taskHandle[i]);
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetObjDescCmpMemset(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    char *kernel_name="test_kernel";
    char str[2][12]={"test_kernel","main_kernel"};
    int32_t ret_value=0;

    ret_value = tivx_obj_desc_strncmp(kernel_name, str[0], strlen(str[0]));
    if(ret_value != 0)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: tivx_obj_desc_strncmp for same string ARGS \n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
       VX_PRINT(VX_ZONE_INFO,"Same string\n");
    }

    ret_value = tivx_obj_desc_strncmp(kernel_name, str[1], strlen(str[1]));
    if(ret_value == 0)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: tivx_obj_desc_strncmp for different string ARGS\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
       VX_PRINT(VX_ZONE_INFO,"Different string\n");
    }

    tivx_obj_desc_memset(str[0],'A',sizeof(str[0]));

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetDebugZone(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum zone = VX_ZONE_INFO;

    if(vx_true_e != tivx_get_debug_zone(zone))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result:VX_ZONE_INFO is cleared\n");
        status = (vx_status)VX_FAILURE;
    }
    if(vx_false_e != tivx_get_debug_zone(VX_ZONE_TARGET))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result:VX_ZONE_TARGET is enabled\n");
        status = (vx_status)VX_FAILURE;
    }
    if(vx_false_e != tivx_get_debug_zone(-1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for the ARG:-1\n");
        status = (vx_status)VX_FAILURE;
    }
    if(vx_false_e != tivx_get_debug_zone(VX_ZONE_MAX))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for the ARG:'VX_ZONE_MAX'\n");
        status = (vx_status)VX_FAILURE;
    }

    tivx_set_debug_zone(-1);
    tivx_set_debug_zone(VX_ZONE_MAX);
    tivx_clr_debug_zone(-1);
    tivx_clr_debug_zone(VX_ZONE_MAX);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelInstance(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum *state = NULL;

    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxSetTargetKernelInstanceContext(NULL,NULL,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxGetTargetKernelInstanceState(NULL,state))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxGetTargetKernelTargetId(NULL,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxGetTargetKernelInstanceContext(NULL,NULL,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }
    tivxGetTargetKernelInstanceBorderMode(NULL,NULL);
    tivxGetTargetKernelInstanceBorderMode(test_kernel,NULL);

    if((vx_bool)vx_false_e != tivxIsTargetKernelInstanceReplicated(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }
    if(NULL != tivxTargetKernelInstanceGetKernel(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetKernelInstance(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t *targetId = (uint32_t *)tivxMemAlloc(sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    vx_border_t *border_mode = (vx_border_t *)tivxMemAlloc(sizeof(vx_border_t),TIVX_MEM_EXTERNAL);

    if((vx_status)VX_SUCCESS != tivxGetTargetKernelTargetId(test_kernel,targetId))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result:Failed to get kernel ID\n");
        status = (vx_status)VX_FAILURE;
    }
    tivxGetTargetKernelInstanceBorderMode(test_kernel,border_mode);

    tivxMemFree((void *)targetId,sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    tivxMemFree((void *)border_mode,sizeof(vx_border_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetGetObjDescElement(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* hit 'elem_idx' > obj_desc_object_array->num_items condition */
    uint16_t elem_idx = 2;
    tivx_obj_desc_t *obj_desc = NULL;

    tivx_obj_desc_object_array_t *obj_desc_object_array = (tivx_obj_desc_object_array_t *)tivxMemAlloc(sizeof(tivx_obj_desc_object_array_t),TIVX_MEM_EXTERNAL);
    obj_desc_object_array->num_items = 1;
    obj_desc = &(obj_desc_object_array->base);
    obj_desc->type = (vx_enum)TIVX_OBJ_DESC_OBJARRAY;

    if(NULL != tivxGetObjDescElement(obj_desc, elem_idx))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result for the ARG: 'elem_idx' > obj_desc_object_array->num_items\n");
        status = (vx_status)VX_FAILURE;
    }
    tivxMemFree((void *) obj_desc_object_array,sizeof(tivx_obj_desc_object_array_t), TIVX_MEM_EXTERNAL);
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetQueryNumTargetKernel(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t *ptr = (uint32_t *)tivxMemAlloc(sizeof(uint32_t), TIVX_MEM_EXTERNAL);

    if((vx_status)VX_SUCCESS != tivxQueryNumTargetKernel(ptr))
    {
       VX_PRINT(VX_ZONE_ERROR,"Invalid result:Failed to query number of target kernels\n");
       status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxQueryNumTargetKernel(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    tivxMemFree((void *)ptr,sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestAddTargetKernelInternal(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 num_target_kernels;
    tivx_target_kernel ttk[TIVX_TARGET_KERNEL_MAX+1] = {NULL};
    vx_uint32 priv_arg = 0;
    int32_t i = 0;
    vx_enum kernel_id = 0;
    char tname[] = {'t', 'i', 'o', 'v', 'x'};

    if(NULL != tivxAddTargetKernel(0,NULL,NULL,NULL,NULL,NULL,NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    tivxQueryNumTargetKernel(&num_target_kernels);

    for (i=num_target_kernels; i<TIVX_TARGET_KERNEL_MAX; i++)
    {
        kernel_id = (vx_enum)(i);
        ttk[i] = tivxAddTargetKernel(kernel_id, tname, process_function, create_function, NULL, NULL, (void *)(&priv_arg));
        if(NULL == ttk[i])
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to add target kernel\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    kernel_id = TIVX_TARGET_KERNEL_MAX;

    /* Trying to allocate TIVX_TARGET_KERNEL_MAX+1 */
    ttk[i] = tivxAddTargetKernel(kernel_id, tname, process_function, create_function, NULL, NULL, (void *)(&priv_arg));
    if(NULL != ttk[i])
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result: TIVX_TARGET_KERNEL_MAX+1 allocation'\n");
        status = (vx_status)VX_FAILURE;
    }

    for (i=num_target_kernels; i<TIVX_TARGET_KERNEL_MAX; i++)
    {
        if((vx_status)VX_SUCCESS != tivxRemoveTargetKernel(ttk[i]))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to remove target kernel\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestRemoveTargetKernel(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 ttkaddress = 0;

    if((vx_status)VX_FAILURE != tivxRemoveTargetKernel(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != tivxRemoveTargetKernel((tivx_target_kernel)(&ttkaddress)))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'&ttkaddress'\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetObjDescAllocFree(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc=NULL;

    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)test_obj_desc->type, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetObjDescQueueCreateRelease(uint8_t id)
{
    uint16_t *objdesc_id = (uint16_t *)tivxMemAlloc(sizeof(uint16_t), TIVX_MEM_EXTERNAL);
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_SUCCESS != ownObjDescQueueCreate(objdesc_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to create object descriptor queue\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS != ownObjDescQueueRelease(objdesc_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to release object descriptor queue\n");
        status = (vx_status)VX_FAILURE;
    }

    tivxMemFree((void *)objdesc_id, sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetObjDescQueueReleaseGetCount(uint8_t id)
{
    uint16_t qid=0;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t val = (uint16_t)TIVX_OBJ_DESC_INVALID;

    if((vx_status)VX_FAILURE != ownObjDescQueueRelease(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueRelease( &val))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'TIVX_OBJ_DESC_INVALID'\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueRelease(&qid))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid queue id\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueGetCount(0,NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetObjDescQueueGetCount(uint8_t id)
{
    uint32_t *count = (uint32_t *)tivxMemAlloc(sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t type = test_obj_desc->type;

    test_obj_desc->type = TIVX_OBJ_DESC_QUEUE;
    if((vx_status)VX_SUCCESS != ownObjDescQueueGetCount(test_obj_desc->obj_desc_id,count))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to get queue count\n");
        status = (vx_status)VX_FAILURE;
    }

    test_obj_desc->type = type;
    tivxMemFree((void *)count, sizeof(uint32_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelInstanceFree(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_ERROR_INVALID_PARAMETERS != ownTargetKernelInstanceFree(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelInstanceGetIndex(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((uint32_t)TIVX_TARGET_KERNEL_INSTANCE_MAX != ownTargetKernelInstanceGetIndex(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *test_params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_enum idx = test_kernel->kernel_id;
    test_kernel->kernel_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if((vx_status)VX_FAILURE != ownTargetKernelCreate(test_kernel,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'test_kernel->kernel_id' = TIVX_OBJ_DESC_INVALID\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);
    test_kernel->kernel_id = idx;

    return status;
}

static vx_status tivxNegativeTestTargetKernelControl(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *test_params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_enum idx = test_kernel->kernel_id;
    test_kernel->kernel_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if( (vx_status)VX_FAILURE != ownTargetKernelControl(test_kernel,1,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'test_kernel->kernel_id' = TIVX_OBJ_DESC_INVALID\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);
    test_kernel->kernel_id = idx;

    return status;
}

static vx_status tivxNegativeTestTargetKernelExecute(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *test_params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_enum idx = test_kernel->kernel_id;
    test_kernel->kernel_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if((vx_status)VX_FAILURE != ownTargetKernelExecute(test_kernel,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'test_kernel->kernel_id' = TIVX_OBJ_DESC_INVALID\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);
    test_kernel->kernel_id = idx;

    return status;
}

static vx_status tivxNegativeTestTargetKernelDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *test_params[TIVX_KERNEL_MAX_PARAMS] = {NULL};
    vx_enum idx = test_kernel->kernel_id;
    test_kernel->kernel_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if((vx_status)VX_FAILURE != ownTargetKernelDelete(test_kernel,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'test_kernel->kernel_id' = TIVX_OBJ_DESC_INVALID\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);
    test_kernel->kernel_id = idx;

    return status;
}

static vx_status tivxNegativeTestTargetKernelGet(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(NULL != ownTargetKernelInstanceAlloc(1,NULL,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: kernel_name = 'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetObjDescQueueEnqueue(uint8_t id)
{
    tivx_obj_desc_queue_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t *objdesc_q_id = (uint16_t *)tivxMemAlloc(sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    if((vx_status)VX_FAILURE != ownObjDescQueueEnqueue(0,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id'\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS != ownObjDescQueueCreate(objdesc_q_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to create object descriptor queue\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_queue_t *)ownObjDescGet(*objdesc_q_id);
        obj_desc->count = TIVX_OBJ_DESC_QUEUE_MAX_DEPTH;

        if((vx_status)VX_FAILURE != ownObjDescQueueEnqueue(*objdesc_q_id,0))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'enqueue count' = TIVX_OBJ_DESC_QUEUE_MAX_DEPTH\n");
            status = (vx_status)VX_FAILURE;
        }

        if((vx_status)VX_SUCCESS != ownObjDescQueueRelease(objdesc_q_id))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to release object descriptor queue\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    tivxMemFree((void *)objdesc_q_id, sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetObjDescQueueExtractBlockedNodes(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_FAILURE != ownObjDescQueueExtractBlockedNodes(0,NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetObjDescQueueAddBlockedNode(uint8_t id)
{
    tivx_obj_desc_queue_t *obj_desc = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t *objdesc_q_id = (uint16_t *)tivxMemAlloc(sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    if((vx_status)VX_FAILURE != ownObjDescQueueAddBlockedNode(0,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id'\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS != ownObjDescQueueCreate(objdesc_q_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to create object descriptor queue\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_queue_t *)ownObjDescGet(*objdesc_q_id);
        obj_desc->blocked_nodes.num_nodes = TIVX_OBJ_DESC_QUEUE_MAX_BLOCKED_NODES;

        if((vx_status)VX_FAILURE != ownObjDescQueueAddBlockedNode(*objdesc_q_id,0))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'blocked node' count = TIVX_OBJ_DESC_QUEUE_MAX_BLOCKED_NODES\n");
            status = (vx_status)VX_FAILURE;
        }
        if((vx_status)VX_SUCCESS != ownObjDescQueueRelease(objdesc_q_id))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to release object descriptor queue\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    tivxMemFree((void *)objdesc_q_id, sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetObjDescQueueDequeue(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t *objdesc_id = (uint16_t *)tivxMemAlloc(sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    if((vx_status)VX_FAILURE != ownObjDescQueueDequeue(0,objdesc_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id'\n");
        status = (vx_status)VX_FAILURE;
    }

    tivxMemFree((void *)objdesc_id, sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelInstanceAlloc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    volatile char kernel[TIVX_KERNEL_MAX_PARAMS] = {1};
    volatile char *kernel_name = kernel;

    if(NULL != ownTargetKernelInstanceAlloc(0, kernel_name, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for kernel_name != 'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetKernelInstanceAllocate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    volatile char kernel[TIVX_KERNEL_MAX_PARAMS] = {1};
    volatile char *kernel_name = kernel;

    /*to hit tmp_kernel_instance->kernel_id == (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID*/
    if(NULL == ownTargetKernelInstanceAlloc(TIVX_TARGET_KERNEL_ID_INVALID , kernel_name, TIVX_TARGET_KERNEL_ID_INVALID ))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'kernel_id = TIVX_TARGET_KERNEL_ID_INVALID'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxBranchTestTargetKernelInstanceAlloc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(NULL != ownTargetKernelInstanceAlloc(test_kernel->kernel_id, NULL, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'kernel_name = NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxBranchTestTargetKernelInstanceAllocate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    volatile char kernel[TIVX_KERNEL_MAX_PARAMS] = {1};
    volatile char *kernel_name = kernel;

    /*to hit tmp_kernel_instance->kernel_id != (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID*/
    if(NULL != ownTargetKernelInstanceAlloc(1, kernel_name, 1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'kernel_id != TIVX_TARGET_KERNEL_ID_INVALID'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxBranchTestTargetKernelInstanceGet(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /*to hit target_kernel_index > TIVX_TARGET_KERNEL_INSTANCE_MAX*/
    if(NULL != ownTargetKernelInstanceGet(TIVX_TARGET_KERNEL_INSTANCE_MAX*3, 1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: target_kernel_index > TIVX_TARGET_KERNEL_INSTANCE_MAX \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}


static vx_status tivxBranchTestTargetKernel(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_FAILURE != ownTargetKernelCreate(test_kernel,NULL,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelCreate: Kernel created with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelExecute(test_kernel,NULL,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelExecute: Kernel executed with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelControl(test_kernel,1,NULL,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelControl: Kernel controlled with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelDelete(test_kernel,NULL,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelDelete: Kernel deleted with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxBranchTestTargetKernelFunc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *test_params[TIVX_KERNEL_MAX_PARAMS] = {NULL};

    if((vx_status)VX_FAILURE != ownTargetKernelCreate(NULL,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelCreate: Kernel created with invalid ARG'\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelExecute(NULL,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelExecute: Kernel executed with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelControl(NULL,1,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelControl: Kernel controlled with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownTargetKernelDelete(NULL,test_params,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: ownTargetKernelDelete: Kernel deleted with invalid ARG\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxObjDescStrncmp(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t ret = 0;
    vx_char  dst[5] = "ABCD";
    vx_char  src[5] = "ABCD";

    /* To fail 'd[i] == 0U' condition*/
    ret = tivx_obj_desc_strncmp((void *)dst, (void *)src, 1U);
    if (0 != ret)
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: tivxObjDescStrncmp: Condition 'd[i] == 0U' passed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestObjDescSend(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dst_target_id = (uint16_t)INVALID_ARG;
    uint16_t obj_desc_id = (uint16_t)1;

    if ((vx_status)VX_ERROR_INVALID_PARAMETERS != ownObjDescSend(dst_target_id, obj_desc_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'invalid argument' dst_target_id, and obj_desc_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestObjDescFree(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc = NULL;
    uint16_t temp_obj_desc_id = 0U;

    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)test_obj_desc->type, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if( (vx_status)VX_ERROR_INVALID_PARAMETERS != ownObjDescFree((tivx_obj_desc_t **)NULL))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' obj_desc\n");
            status = (vx_status)VX_FAILURE;
        }

        temp_obj_desc_id = obj_desc->obj_desc_id;
        obj_desc->obj_desc_id = (uint16_t)INVALID_ARG;

        if( (vx_status)VX_ERROR_INVALID_PARAMETERS != ownObjDescFree((tivx_obj_desc_t **)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'INVALID_ARG' obj_desc\n");
            status = (vx_status)VX_FAILURE;
        }

        obj_desc->obj_desc_id = temp_obj_desc_id;

        if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestObjDescIsValidType(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc = NULL;
    uint16_t temp_obj_desc_id = 0U;

    if( (vx_bool)vx_false_e != ownObjDescIsValidType(obj_desc, TIVX_OBJ_DESC_INVALID))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL' obj_desc and 'invalid_type' obj_type\n");
        status = (vx_status)VX_FAILURE;
    }

    obj_desc = ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_IMAGE, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        temp_obj_desc_id = obj_desc->obj_desc_id;
        obj_desc->obj_desc_id = (uint16_t)INVALID_ARG;

        if( (vx_bool)vx_false_e != ownObjDescIsValidType(obj_desc, TIVX_OBJ_DESC_INVALID))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'obj_desc->obj_desc_id' and 'invalid type' obj_type\n");
            status = (vx_status)VX_FAILURE;
        }

        obj_desc->obj_desc_id = temp_obj_desc_id;

        if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestGetObjDescList(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t obj_desc_id[1] = {(uint16_t)-1};
    uint32_t num_desc_id = 1U;

    tivxGetObjDescList(obj_desc_id, (tivx_obj_desc_t **)NULL, num_desc_id);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestDescStrncpy(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_char  dst[5] = "";
    vx_char  src[5] = "ABCD";

    /* To fail 'i<(size-1U)' "for loop" condition*/
    tivx_obj_desc_strncpy((void *)dst, (void *)src, 1U);
    if (dst[0] == src[0])
    {
        VX_PRINT(VX_ZONE_ERROR,"ERROR: tivx_obj_desc_strncpy: Condition 'i<(size-1U)' passed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestDescStrncmpDelim(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_char  dst[5] = "ABCD";
    vx_char  src[5] = "ABCD";
    uint32_t size = 0;
    int32_t ret_val = 0;

    /* To fail 'i<size' "for loop" condition*/
    ret_val = tivx_obj_desc_strncmp_delim(dst, src, size, 'A');
    if(0 != ret_val)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: tivx_obj_desc_strncmp_delim for different ARGS\n");
        status = (vx_status)VX_FAILURE;
    }

    size = 2;
    /* To fail 'd[i] == (uint8_t)delim' and 's[i] == (uint8_t)delim' condition*/
    ret_val = tivx_obj_desc_strncmp_delim(dst, src, size, 'E');
    if(0 != ret_val)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: tivx_obj_desc_strncmp_delim for different ARGS\n");
        status = (vx_status)VX_FAILURE;
    }

    /* To fail 'd[i] != (uint8_t)delim' and 's[i] != (uint8_t)delim' condition*/
    ret_val = tivx_obj_desc_strncmp_delim(dst, src, size, 'B');
    if(0 != ret_val)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: tivx_obj_desc_strncmp_delim for different ARGS\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestObjDescSend(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dst_target_id = (uint16_t)-1;
    uint16_t obj_desc_id = (uint16_t)-1;

    if ((vx_status)VX_FAILURE != ownObjDescSend(dst_target_id, obj_desc_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'invalid argument' dst_target_id, and obj_desc_id\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestGetObjDescElement(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc = NULL;
    tivx_obj_desc_object_array_t *obj_desc_object_array = NULL;

    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)test_obj_desc->type, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc_object_array = (tivx_obj_desc_object_array_t *)obj_desc;
        obj_desc_object_array->num_items = 1;
        obj_desc->type = (vx_enum)TIVX_OBJ_DESC_OBJARRAY;

        if (NULL == tivxGetObjDescElement(obj_desc, (uint16_t)0))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result for the ARG: 'elem_idx' < obj_desc_object_array->num_items\n");
            status = (vx_status)VX_FAILURE;
        }

        obj_desc->type = (uint16_t)INVALID_ARG;
        obj_desc->scope_obj_desc_id = (uint16_t)TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST;
        if (NULL != tivxGetObjDescElement(obj_desc, (uint16_t)INVALID_ARG))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result for the ARG: obj_desc->scope_obj_desc_id\n");
            status = (vx_status)VX_FAILURE;
        }

        if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#ifndef PC
static vx_status tivxNegativeTestTargetGetHandleAndDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum cpu_id = tivxGetSelfCpuId();
    vx_enum target_id = 0;

    switch (cpu_id)
    {
        #if defined(SOC_J721E)
        case TIVX_CPU_ID_DSP_C7_1:
        #endif
        case TIVX_CPU_ID_DSP1:
            #if defined(AM62A)
            target_id = (vx_enum)TIVX_TARGET_ID_MCU1_0;
            #else
            target_id = (vx_enum)TIVX_TARGET_ID_MCU2_0;
            #endif
            break;
        #if defined(AM62A)
        case TIVX_CPU_ID_MCU1_0:
        #else
        case TIVX_CPU_ID_MCU2_0:
        #endif
            target_id = (vx_enum)TIVX_TARGET_ID_MPU_2;
            break;
        case TIVX_CPU_ID_MPU_0:
            #if defined(SOC_J721S2)
            target_id = (vx_enum)TIVX_TARGET_ID_DSP1;
            #elif defined(SOC_J721E)
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_1;
            #else
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_2;
            #endif
            break;
    }
    if ((vx_status)VX_ERROR_INVALID_VALUE != ownTargetDelete((vx_enum)target_id))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid result returned for ARG: 'INVALID_ARG' target id\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_ERROR_INVALID_VALUE != ownTargetDelete((vx_enum)INVALID_ARG))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid result returned for ARG: 'INVALID_ARG' target id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetTriggerNode(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc=NULL;
    uint16_t obj_desc_id_temp;

    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_NODE, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc_id_temp = obj_desc->obj_desc_id;
        obj_desc->obj_desc_id = (uint16_t)INVALID_ARG;
        ownTargetTriggerNode(obj_desc->obj_desc_id);
        obj_desc->obj_desc_id = obj_desc_id_temp;
        if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetQueueObjDesc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((vx_status)VX_ERROR_INVALID_PARAMETERS != ownTargetQueueObjDesc((vx_enum)INVALID_ARG, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestObjDescAlloc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum cpu_id = tivxGetSelfCpuId();
    vx_enum target_id = 0;

    switch (cpu_id)
    {
        case TIVX_CPU_ID_DSP1:
            #if defined (SOC_J721E) || defined (SOC_J721S2)
            target_id = (vx_enum)TIVX_TARGET_ID_DSP1;
            #else
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_2;
            #endif
            break;
        #if defined(AM62A)
        case TIVX_CPU_ID_MCU1_0:
            target_id = (vx_enum)TIVX_TARGET_ID_MCU1_0;
            break;
        #else
        case TIVX_CPU_ID_MCU2_0:
            target_id = (vx_enum)TIVX_TARGET_ID_MCU2_0;
            break;
        #endif
        case TIVX_CPU_ID_MPU_0:
            target_id = (vx_enum)TIVX_TARGET_ID_MPU_2;
            break;
        #if defined(SOC_J721E)
        case TIVX_CPU_ID_DSP_C7_1:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_1;
            break;
        #endif
    }

    if (VX_ERROR_NO_RESOURCES != ownTargetCreate(target_id, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for ARG:'target_id' Target ID \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetGetHandle(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((vx_status)VX_ERROR_NO_RESOURCES != ownTargetCreate((vx_enum)INVALID_ARG, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG:'NULL'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

/*Test case to fail the condition 'if(0 != ownObjDescIsValidType((tivx_obj_desc_t*)data_ref_q_obj_desc, TIVX_OBJ_DESC_DATA_REF_Q)'
 *inside the function ownTargetNodeDescAcquireAllParameters
 */
static vx_status tivxNegativeTestTargetNodeDescAcquireAllParameter(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    vx_bool test_node_blocked;
    uint16_t obj_desc_id_t = 1;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

    /*to set is_prm_data_ref_q_flag so that tivxFlagIsBitSet() returns '1' inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;

    if(test_obj_desc_node != NULL)
    {
        ownTargetNodeDescAcquireAllParameters(test_obj_desc_node, prm_obj_desc_id, &test_node_blocked, 0);

        /*prm_obj_desc_id[0] is updated inside ownTargetNodeDescAcquireAllParameters() */
        if(prm_obj_desc_id[0] != (vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for prm_obj_desc_id[0]\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to get test_obj_desc_node\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*Test case to fail 'if ((vx_bool)vx_false_e == is_pipeup)' in ownTargetNodeDescAcquireParameter()*/
static vx_status tivxBranchTestTargetNodeDescAcquireParameter(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    vx_bool test_node_blocked;
    vx_bool test_pipe = (vx_bool)vx_true_e;
    tivx_obj_desc_t *obj_desc =NULL;
    uint16_t obj_desc_id_t = 1;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(TIVX_OBJ_DESC_DATA_REF_Q, NULL);
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

    /*to set is_prm_data_ref_q_flag so that tivxFlagIsBitSet() returns '1' inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;

    /*To create valid obj_desc of TIVX_OBJ_DESC_DATA_REF_Q type*/
    test_obj_desc_node->data_ref_q_id[0]= obj_desc->obj_desc_id;
    test_obj_desc_node->is_prm_input = 0;

    if(obj_desc !=NULL && test_obj_desc_node != NULL)
    {
        ownTargetNodeDescAcquireAllParameters(test_obj_desc_node, prm_obj_desc_id, &test_node_blocked, test_pipe);

        /*test_node_blocked is updated inside ownTargetNodeDescAcquireParameter()*/
        if(test_node_blocked != vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for test_node_blocked\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*Test case to fail 'if((vx_status)VX_SUCCESS == ownObjDescQueueAddBlockedNode()' in ownTargetNodeDescAcquireParameter()*/
static vx_status tivxBranchTestTargetNodeDescAcquireParam(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    vx_bool test_node_blocked;
    vx_bool test_pipe = (vx_bool)vx_false_e;
    tivx_obj_desc_t *obj_desc =NULL;
    uint16_t obj_desc_id_t = 1;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(TIVX_OBJ_DESC_DATA_REF_Q, NULL);
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

    /*to set is_prm_data_ref_q_flag so that tivxFlagIsBitSet() returns '1' inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;

    /*To create valid obj_desc of TIVX_OBJ_DESC_DATA_REF_Q type*/
    test_obj_desc_node->data_ref_q_id[0]= obj_desc->obj_desc_id;
    test_obj_desc_node->is_prm_input = 0;

    if(obj_desc !=NULL && test_obj_desc_node != NULL)
    {
        ownTargetNodeDescAcquireAllParameters(test_obj_desc_node, prm_obj_desc_id, &test_node_blocked, test_pipe);

        /*test_node_blocked is updated inside ownTargetNodeDescAcquireParameter()*/
        if(test_node_blocked != vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for test_node_blocked\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*Test case to create data_ref_q_obj_desc which is not corresponding to TIVX_OBJ_DESC_DATA_REF_Q
 *in ownTargetNodeDescReleaseAllParameters.
 */
static vx_status tivxBranchTestTargetNodeDescReleaseAllParameter(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    uint16_t obj_desc_id_t = 1; /*Random value*/
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

    /*to set is_prm_data_ref_q_flag so that tivxFlagIsBitSet() returns '1' inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;
    /*Random id passed to test_obj_desc_node->data_ref_q_id[0]*/
    test_obj_desc_node->data_ref_q_id[0]=obj_desc_id_t;
    test_obj_desc_node->is_prm_input = 0;

    if(test_obj_desc_node != NULL)
    {
        ownTargetNodeDescReleaseAllParameters(test_obj_desc_node, prm_obj_desc_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to get test_obj_desc_node \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/* Test case to make 'data_ref_q_obj_desc->num_in_nodes = 0'  inside ownTargetNodeDescReleaseParameter() */
static vx_status tivxBranchTargetNodeDescReleaseParameter(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    tivx_obj_desc_t *obj_desc =NULL;
    uint16_t obj_desc_id_t = 1;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(TIVX_OBJ_DESC_DATA_REF_Q, NULL);
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

    /*to set is_prm_data_ref_q_flag for returning true from tivxFlagIsBitSet inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;

    /*To create valid obj_desc of TIVX_OBJ_DESC_DATA_REF_Q type*/
    test_obj_desc_node->data_ref_q_id[0]= obj_desc->obj_desc_id;

    /*
     *prm_input should be a value other than 0(vx_false_e) and 1(vx_true_e)
     *to validate 'data_ref_q_obj_desc->num_in_nodes' inside ownTargetNodeDescReleaseParameter()
     */
    test_obj_desc_node->is_prm_input = 2;

    /*To make obj_desc = NULL inside ownTargetNodeDescReleaseParameter() */
    prm_obj_desc_id[0] = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if(obj_desc !=NULL && test_obj_desc_node != NULL)
    {
        ownTargetNodeDescReleaseAllParameters(test_obj_desc_node, prm_obj_desc_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*Test case to fail ownObjDescQueueEnqueue()inside ownTargetNodeDescReleaseParameter()*/
static vx_status tivxBranchTargetNodeDescReleaseParam(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_node_t *test_obj_desc_node = NULL;
    tivx_obj_desc_t *obj_desc =NULL;
    uint16_t obj_desc_id_t = 1;
    uint16_t prm_obj_desc_id[TIVX_KERNEL_MAX_PARAMS]={0};
    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc(TIVX_OBJ_DESC_DATA_REF_Q, NULL);
    test_obj_desc_node = (tivx_obj_desc_node_t *)ownObjDescGet(obj_desc_id_t);

   /*to set is_prm_data_ref_q_flag so that tivxFlagIsBitSet() returns '1' inside ownTargetNodeDescReleaseAllParameters() */
    test_obj_desc_node->is_prm_data_ref_q = 1;
    test_obj_desc_node->num_params =1;

    /*To create valid obj_desc of TIVX_OBJ_DESC_DATA_REF_Q type*/
    test_obj_desc_node->data_ref_q_id[0]= obj_desc->obj_desc_id;
    test_obj_desc_node->is_prm_input = 0;

    if(obj_desc !=NULL && test_obj_desc_node != NULL)
    {
        ownTargetNodeDescReleaseAllParameters(test_obj_desc_node, prm_obj_desc_id);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestObjDescAllocAndDescQueueCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t *obj_desc_id = (uint16_t *)tivxMemAlloc(sizeof(uint16_t), TIVX_MEM_EXTERNAL);
    int i, j;

   /* Maxing out the OBJECT DESCRIPTORS */
    for (i = 0; i < TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST; i++)
    {
        g_obj_desc[i] = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)test_obj_desc->type, NULL);
        if (NULL == g_obj_desc[i])
        {
            if ((vx_status)VX_FAILURE != ownObjDescQueueCreate(obj_desc_id))
            {
                VX_PRINT(VX_ZONE_ERROR,"Create a object descriptor queue\n");
                status = (vx_status)VX_FAILURE;
            }
            break;
        }
    }

    for (j = 0; j < i; j++)
    {
        if (NULL == g_obj_desc[j])
        {
            break;
        }
        ownObjDescFree((tivx_obj_desc_t**)&g_obj_desc[j]);
    }

    tivxMemFree((void *)obj_desc_id, sizeof(uint16_t), TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#if defined(A72) || defined(A53)
static vx_status tivxTestTargetPlatformGetEnv(uint8_t id)
{
    extern char *tivxPlatformGetEnv(char *env_var);

    vx_status status = (vx_status)VX_SUCCESS;
    const char *env1 = "/test_data/";
    const char *env2 = " ";

    if(env1 != tivxPlatformGetEnv("VX_TEST_DATA_PATH"))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to get valid test data path\n");
        status = (vx_status)VX_FAILURE;
    }
    if(env2 != tivxPlatformGetEnv("INVALID_PATH"))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid environment variable\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestAppIpcGetIpcCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MCU1_0;

    if (IPC_MP_INVALID_ID == appIpcGetIpcCpuId(app_cpu_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for app_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestAppIpcGetAppCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    const char *name  = appIpcGetCpuName(APP_IPC_CPU_MCU1_0);

    if ((vx_status)VX_SUCCESS == appIpcGetAppCpuId((char *)name))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;

}

static vx_status tivxTestAppIpcGetCpuName(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MPU1_0;
    const char *name1 = "";
    const char *name2  = appIpcGetCpuName(APP_IPC_CPU_MPU1_0);

    name1 = appIpcGetCpuName(app_cpu_id);
    if((vx_status)VX_SUCCESS != strncmp(name1,name2,strlen(name2)))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name1 and name2\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;

}
#endif

#ifndef PC
static vx_status tivxNegativeTestTargetPlatformDeleteTargetId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxPlatformDeleteTargetId(-1);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetPlatformCreateTargetId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum cpu_id = tivxGetSelfCpuId();
    vx_enum target_id = 0;

    switch (cpu_id)
    {
        case TIVX_CPU_ID_DSP1:
            #if defined(SOC_J721S2)
            target_id = (vx_enum)TIVX_TARGET_ID_DSP1;
            #elif defined(SOC_J721E)
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_1;
            #else
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_2;
            #endif
            break;
        #if defined(AM62A)
        case TIVX_CPU_ID_MCU1_0:
        #else
        case TIVX_CPU_ID_MCU2_0:
        #endif
            target_id = (vx_enum)TIVX_TARGET_ID_MCU2_0;
            break;
        case TIVX_CPU_ID_MPU_0:
            target_id = (vx_enum)TIVX_TARGET_ID_MPU_0;
            break;
        #if defined(SOC_J721E)
        case TIVX_CPU_ID_DSP_C7_1:
            target_id = (vx_enum)TIVX_TARGET_ID_DSP_C7_1;
            break;
        #endif
    }

    /* To fail ownTargetCreate() inside  tivxPlatformCreateTargetId API */
    tivxPlatformCreateTargetId(target_id, 0u, "TIVX_CPU0", 8U);

    /* To fail ownTargetGetCpuId(target_id) == tivxGetSelfCpuId() check*/
    tivxPlatformCreateTargetId((vx_enum)INVALID_ARG, 0u, "INVALID", 8U);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppIpcIsCpuEnabled(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cpu_id = APP_IPC_CPU_MAX;

    /* '0' in this case is failure */
    if ((uint32_t)0 != appIpcIsCpuEnabled(cpu_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTestTargetTaskSetDefaultCreateParams(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxTaskSetDefaultCreateParams(NULL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetTaskDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_task task;

    if((vx_status)VX_FAILURE != tivxTaskDelete(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }
    task.tsk_handle =NULL;
    if((vx_status)VX_FAILURE != tivxTaskDelete(&task))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task handle\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetTaskCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_task task;

    if((vx_status)VX_FAILURE != tivxTaskCreate(NULL, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != tivxTaskCreate(&task, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' params\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
static vx_status tivxAppMemPrintMemAllocInfo(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    appMemPrintMemAllocInfo();

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppMemGetNumAllocs(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* For rtos implementation, appMemGetNumAllocs()
    is not valid and just return -1 */
    if ((uint32_t)(-1) != appMemGetNumAllocs())
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for appMemGetNumAllocs()\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppMemUnMap(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *virt_ptr = NULL;
    uint32_t size = 0;

    /* nothing to do in rtos */
    if ((vx_status)VX_SUCCESS != appMemUnMap(virt_ptr, size))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for appMemUnMap()\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#if !defined (MCU_PLUS_SDK)

static vx_status tivxAppIpcGetIpcCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MCU1_1;

    if (IPC_MP_INVALID_ID == appIpcGetIpcCpuId(app_cpu_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for app_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#endif

static vx_status tivxAppIpcGetAppCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    const char *name  = appIpcGetCpuName(APP_IPC_CPU_MPU1_0);

    if ((vx_status)VX_SUCCESS  != appIpcGetAppCpuId((char *)name))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppIpcGetCpuName(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MPU1_0;
    const char *name1 = "";
    const char *name2  = appIpcGetCpuName(APP_IPC_CPU_MPU1_0);

    name1 = appIpcGetCpuName(app_cpu_id);
    if((vx_status)VX_SUCCESS  != strcmp(name1,name2))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name1 and name2\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcHwLockAcquire(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t hw_lock_id = APP_IPC_HW_SPIN_LOCK_MAX;
    uint32_t timeout = 0;

    if ((vx_status)VX_FAILURE != appIpcHwLockAcquire(hw_lock_id, timeout))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for hw_lock_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcHwLockRelease(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t hw_lock_id = APP_IPC_HW_SPIN_LOCK_MAX;

    if ((vx_status)VX_FAILURE != appIpcHwLockRelease(hw_lock_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for hw_lock_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcGetIpcCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MAX;

    if (IPC_MP_INVALID_ID != appIpcGetIpcCpuId(app_cpu_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for app_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcGetAppCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    const char *name  = "invalid";

    if (APP_IPC_CPU_INVALID  != appIpcGetAppCpuId((char *)name))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcGetCpuName(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MAX;
    const char *name1 = "";
    const char *name2  = "invalid";

    name1 = appIpcGetCpuName(app_cpu_id);
    if((vx_status)VX_SUCCESS != strncmp(name1,name2,strlen(name2)))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name1 and name2\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcSendNotifyPort(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dest_cpu_id = APP_IPC_CPU_MAX;
    uint32_t payload = 0;
    uint32_t port_id = 0;

    if ((vx_status)VX_FAILURE != appIpcSendNotifyPort(dest_cpu_id, payload, port_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for dest_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppIpcSendNotify(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dest_cpu_id = APP_IPC_CPU_MAX;
    uint32_t payload = 0;

    if ((vx_status)VX_FAILURE != appIpcSendNotify(dest_cpu_id, payload))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for dest_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppMemAlloc(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id = APP_MEM_HEAP_MAX;
    uint32_t size = 0;
    uint32_t align = 0;

    if (NULL != appMemAlloc(heap_id, size, align))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for heap_id\n");
        status = (vx_status)VX_FAILURE;
    }

    heap_id = APP_MEM_HEAP_L3;
    if (NULL != appMemAlloc(heap_id, size, align))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for heap_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppMemResetScratchHeap(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id = APP_MEM_HEAP_MAX;

    if ((vx_status)VX_FAILURE != appMemResetScratchHeap(heap_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for heap_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppMemFree(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id = APP_MEM_HEAP_MAX;
    void *ptr = NULL;
    uint32_t size = 0u;

    if ((vx_status)VX_FAILURE != appMemFree(heap_id, ptr, size))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for heap_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppMemStats(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id = APP_MEM_HEAP_MAX;
    app_mem_stats_t *stats = NULL;

    if ((vx_status)VX_FAILURE != appMemStats(heap_id, stats))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for heap_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppRtosSemaphoreParamsInit(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_params_t *params = NULL;

    appRtosSemaphoreParamsInit(params);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppRtosSemaphoreDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_handle_t *semhandle = NULL;
    app_rtos_semaphore_handle_t semhandle_temp = NULL;

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE != appRtosSemaphoreDelete(semhandle))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE != appRtosSemaphoreDelete(&semhandle_temp))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for semhandle_temp\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppRtosSemaphorePost(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_handle_t semhandle = NULL;

    if ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS != appRtosSemaphorePost(semhandle))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppRtosTaskParamsInit(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_task_params_t *params = NULL;

    appRtosTaskParamsInit(params);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeAppRtosTaskDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_task_handle_t *handle = NULL;
    app_rtos_task_handle_t handle_temp = NULL;

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE != appRtosTaskDelete(handle))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE != appRtosTaskDelete(&handle_temp))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for semhandle_temp\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeappRtosSemaphorePend(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_handle_t semhandle = NULL;
    app_rtos_semaphore_params_t params;
    uint32_t timeout = APP_RTOS_SEMAPHORE_NO_WAIT;

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE != appRtosSemaphorePend(semhandle, timeout))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'semhandle' = NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    params.mode = APP_RTOS_SEMAPHORE_MODE_COUNTING;
    params.maxValue = 0x02U;
    params.initValue = 0U;

    semhandle = appRtosSemaphoreCreate(params);
    if (NULL == semhandle)
    {
        VX_PRINT(VX_ZONE_ERROR,"Semaphore creation failed\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((app_rtos_status_t)APP_RTOS_STATUS_TIMEOUT != appRtosSemaphorePend(semhandle, timeout))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'timeout' = APP_RTOS_SEMAPHORE_NO_WAIT\n");
        status = (vx_status)VX_FAILURE;
    }

    timeout = 1;
    if ((app_rtos_status_t)APP_RTOS_STATUS_TIMEOUT != appRtosSemaphorePend(semhandle, timeout))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'timeout' = INVALID_ARG\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE == appRtosSemaphoreDelete(&semhandle))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to delete semaphore\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppRtosSemaphoreReset(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_handle_t semhandle = NULL;

    if ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS != appRtosSemaphoreReset(semhandle))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'semhandle' = NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeappRtosTaskCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    const app_rtos_task_params_t *params = NULL;

    if (NULL != appRtosTaskCreate(params))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'params' = NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppRtosTaskSleep(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t timeout = 100U;

    appRtosTaskSleep(timeout);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxAppRtosTaskYield(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    appRtosTaskYield();

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTestTargetEventCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_FAILURE != tivxEventCreate(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetMutex(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_FAILURE != tivxMutexCreate(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != tivxMutexDelete(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxMutexLock(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != tivxMutexUnlock(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetPlatformGetShmSize(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t shm_size =0;

    if((uint32_t)0 != tivxPlatformGetShmSize(&shm_size))
    {
        VX_PRINT(VX_ZONE_ERROR,"Unable to get TIOVX object descriptor memory carveout\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#ifndef PC
static vx_status tivxTestTargetEnabled(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(vx_true_e != tivxIsTargetEnabled(TIVX_TARGET_MCU2_0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result\n");
        status = (vx_status)VX_FAILURE;
    }
    if(vx_false_e != tivxIsTargetEnabled(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result\n");
        status = (vx_status)VX_FAILURE;
    }
    if(vx_false_e != tivxIsTargetEnabled(INVALID_TARGET))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTestTargetEvent(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_status)VX_SUCCESS == tivxEventDelete(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' event\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS == tivxEventPost(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' event\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS == tivxEventWait(NULL,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' event\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS == tivxEventClear(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' event\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetEventClear(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_event event;

    if((vx_status)VX_SUCCESS != tivxEventCreate(&event))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to create event\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS != tivxEventClear(event))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to clear event\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS != tivxEventDelete(&event))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to delete event\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetEventWait(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_event event;
    if((vx_status)VX_SUCCESS != tivxEventCreate(&event))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to create event\n");
        status = (vx_status)VX_FAILURE;
    }
/* To hit micro >= 1000000LLU condition present inside linux tivx_event.c file*/
#if defined (A72) || defined (A53) || defined PC
    VX_PRINT(VX_ZONE_INFO,"---Testing  tivxEventWait API for timeout=1000--- \n");
    if((vx_status)TIVX_ERROR_EVENT_TIMEOUT != tivxEventWait(event,1000))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for timeout value = 1000 \n");
        status = (vx_status)VX_FAILURE;
    }
#endif

    if((vx_status)TIVX_ERROR_EVENT_TIMEOUT!= tivxEventWait(event,TIVX_EVENT_TIMEOUT_NO_WAIT))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for timeout value = TIVX_EVENT_TIMEOUT_NO_WAIT\n");
        status = (vx_status)VX_FAILURE;
    }
/* To hit else condition inside rtos tivxEventWait() by passing different timeout value than TIVX_EVENT_TIMEOUT_WAIT_FOREVER or TIVX_EVENT_TIMEOUT_NO_WAIT */
    if((vx_status)TIVX_ERROR_EVENT_TIMEOUT != tivxEventWait(event,2))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for timeout value = 2\n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_SUCCESS != tivxEventDelete(&event))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to delete event\n");
        status = (vx_status)VX_FAILURE;
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#ifndef PC
static vx_status tivxTestTargetPlatformSetHostTargetId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxPlatformSetHostTargetId(TIVX_TARGET_ID_MPU_0);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTestTargetPlatformRtos(uint8_t id)
{
    #define TEST_TARGET_ID_INVALID -2
    vx_status status = (vx_status)VX_SUCCESS;
    char target_name[64];
    char test_char[] = "UNKNOWN";

    if((vx_enum)TIVX_TARGET_ID_INVALID != ownPlatformGetTargetId(NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' target_name \n");
        status = (vx_status)VX_FAILURE;
    }
    ownPlatformGetObjDescTableInfo(NULL);

    ownPlatformGetTargetName(TIVX_TARGET_ID_INVALID, target_name);
    if(strncmp(test_char, target_name, sizeof(test_char)) != 0)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for target_id = TIVX_TARGET_ID_INVALID \n");
        status = (vx_status)VX_FAILURE;
    }
    /*To fail i < TIVX_PLATFORM_MAX_TARGETS for-loop condition by passing invalid id other than TIVX_TARGET_ID_INVALID*/
    ownPlatformGetTargetName(TEST_TARGET_ID_INVALID, target_name);
    if(strncmp(test_char, target_name, sizeof(test_char)) != 0)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for target_id = TIVX_TARGET_ID_INVALID \n");
        status = (vx_status)VX_FAILURE;
    }

    ownPlatformGetLogRtShmInfo(NULL,NULL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetPlatformSystemLock(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    ownPlatformSystemLock(TIVX_PLATFORM_LOCK_LOG_RT);
    ownPlatformSystemUnlock(TIVX_PLATFORM_LOCK_LOG_RT);

    /* To hit the else condition */
    ownPlatformSystemLock((vx_enum)1);
    ownPlatformSystemUnlock((vx_enum)1);

    /* To fail condition lock_id < ( vx_enum ) TIVX_PLATFORM_LOCK_MAX */
    ownPlatformSystemLock((vx_enum) TIVX_PLATFORM_LOCK_MAX + 1);
    ownPlatformSystemUnlock((vx_enum) TIVX_PLATFORM_LOCK_MAX + 1);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestTargetInitHost(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* To fail tivxInitLocal() and tivxDeinitLocal initial checks*/
    tivxInit();
    tivxDeInit();
    /* To fail tivxHostInitLocal() and tivxHostDeinitLocal initial checks*/
#if defined (A72) || defined (A53)
    tivxHostInit();
    tivxHostDeInit();
#endif

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
static vx_status tivxNegativeAppRtosSemaphoreCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    app_rtos_semaphore_handle_t handle;
    app_rtos_semaphore_params_t Params;

    appRtosSemaphoreParamsInit(&Params);
    Params.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
    Params.initValue = -1U;

    handle = appRtosSemaphoreCreate(Params);
    if (NULL != handle)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'Params.initValue' = '-1u'\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestMemStats(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t heap_id = (uint32_t)APP_MEM_HEAP_MAX;
    int32_t mem_heap_region = TIVX_MEM_EXTERNAL;
    app_mem_stats_t mem_stats;

    for (mem_heap_region = 0; mem_heap_region <= (uint32_t)APP_MEM_HEAP_MAX; mem_heap_region++)
    {
        if ((vx_bool)vx_false_e != tivxMemRegionQuery ((vx_enum)mem_heap_region))
        {
            if ((vx_status)VX_FAILURE != appMemStats(heap_id, &mem_stats))
            {
                VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: 'heap_id' = APP_MEM_HEAP_MAX\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

#ifndef PC
static vx_status tivxNegativeTestTargetIpcSendMsg(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum cpu_id;

    cpu_id = tivxGetSelfCpuId();

    /* To fail ownIpcSendMsg() by passing invalid parameters */
    if((vx_status)VX_ERROR_INVALID_PARAMETERS != ownIpcSendMsg(cpu_id, 0, cpu_id, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: host port ID=0 & payload=0 \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTaskMain(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_task task;
    tivx_task_create_params_t taskParams;
    tivxTaskSetDefaultCreateParams(&taskParams);

    taskParams.task_main = NULL;

    if((vx_status)VX_FAILURE != tivxTaskCreate(&task, &taskParams))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'NULL' task_main\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*To hit uncovered regions in /psdk_j7/rtos/tivx_queue.c*/
#if defined(R5F) || defined(C7X_FAMILY) || defined(C66)
static vx_status tivxTestQueueCreateDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_queue que[10];
    tivx_queue *test_queue=&que[0];
    uintptr_t mem[10];
    uintptr_t *queue_memory = &mem[0];
    uint32_t max_elements = 1;
    uintptr_t data;

    if(VX_SUCCESS != tivxQueueCreate(test_queue,max_elements,queue_memory,TIVX_QUEUE_FLAG_BLOCK_ON_PUT))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxQueueCreate failed\n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->count=0;
    if(vx_true_e !=  tivxQueueIsEmpty((const tivx_queue *)test_queue))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue is not empty\n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->block_rd =NULL;
    test_queue->flags = TIVX_QUEUE_FLAG_BLOCK_ON_GET;
    if(VX_FAILURE != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Event posted for queue in tivxQueuePut() \n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"blocking on que put is not disabled\n");
        status = (vx_status)VX_FAILURE;
    }

    if(VX_SUCCESS != tivxQueuePeek((const tivx_queue *)test_queue,&data))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue Peek failed\n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->flags = TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    if(VX_SUCCESS != tivxQueueDelete(test_queue))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue Delete failed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestQueuePutGetDelete(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_queue que[10];
    tivx_queue *test_queue=&que[0];
    uintptr_t mem[10];
    uintptr_t *queue_memory = &mem[0];
    uint32_t max_elements =1;
    uintptr_t data;

    if(VX_SUCCESS != tivxQueueCreate(test_queue,max_elements,queue_memory,TIVX_QUEUE_FLAG_BLOCK_ON_GET))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue is not Created \n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->count=0;
    test_queue->flags = TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    if(VX_SUCCESS != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Event posted for queue\n");
        status = (vx_status)VX_FAILURE;
    }

    if(VX_FAILURE != tivxQueuePut(test_queue,10,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"timeout is not TIVX_EVENT_TIMEOUT_NO_WAIT\n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->block_wr =NULL;
    test_queue->flags = TIVX_QUEUE_FLAG_BLOCK_ON_PUT;
    if(VX_FAILURE != tivxQueueGet(test_queue,&data,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Event posted for queue in tivxQueueGet()\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxQueueGet(test_queue,&data,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"timeout for tivxQueueGet is not TIVX_EVENT_TIMEOUT_NO_WAIT\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxQueueGet(test_queue,&data,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"Blocking on que is not  disabled\n");
        status = (vx_status)VX_FAILURE;
    }
    test_queue->flags = TIVX_QUEUE_FLAG_BLOCK_ON_GET;
    if(VX_SUCCESS != tivxQueueDelete(test_queue))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue Delete failed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

#endif

#ifndef PC
#if defined(LINUX)
static vx_status tivxNegativeTaskAppIpcGetHostPortId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t port_id = 0u;
    uint32_t cpu_id = APP_IPC_CPU_MAX;

    /* '0' in this case is failure */
    port_id = appIpcGetHostPortId(cpu_id);
    if((uint32_t)0 != port_id)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_CPU_MAX' cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTaskAppIpcGetIpcCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MAX;

    if((vx_status)VX_FAILURE != (vx_status)appIpcGetIpcCpuId(app_cpu_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_CPU_MAX' app_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAppIpcGetAppCpuId(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    const char *name  = "invalid";

    if(APP_IPC_CPU_INVALID  != appIpcGetAppCpuId((char *)name))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'invalid' name\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAppIpcGetCpuName(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t app_cpu_id = APP_IPC_CPU_MAX;
    const char *name1 = "";
    const char *name2  = "invalid";

    name1 = appIpcGetCpuName(app_cpu_id);
    if((vx_status)VX_SUCCESS != strncmp(name1,name2,strlen(name2)))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid Result returned for name1 and name2\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAappIpcHwLockAcquire(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t hw_lock_id = APP_IPC_HW_SPIN_LOCK_MAX;
    uint32_t timeout = 0;

    if((vx_status)VX_FAILURE != appIpcHwLockAcquire(hw_lock_id, timeout))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_HW_SPIN_LOCK_MAX' hw_lock_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAppIpcHwLockRelease(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t hw_lock_id = APP_IPC_HW_SPIN_LOCK_MAX;

    if((vx_status)VX_FAILURE != appIpcHwLockRelease(hw_lock_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_HW_SPIN_LOCK_MAX' hw_lock_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAppIpcSendNotifyPort(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dest_cpu_id = APP_IPC_CPU_MAX;
    uint32_t payload = 0;
    uint32_t port_id = 0;

    if((vx_status)VX_FAILURE != appIpcSendNotifyPort(dest_cpu_id, payload, port_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_CPU_MAX' dest_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTaskAppIpcSendNotify(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t dest_cpu_id = APP_IPC_CPU_MAX;
    uint32_t payload = 0;

    if((vx_status)VX_FAILURE != appIpcSendNotify(dest_cpu_id, payload))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for 'APP_IPC_CPU_MAX' dest_cpu_id\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

static vx_status tivxNegativeTestMutexMaxOut(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int i,j = 0;
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
#if defined(MCU_PLUS_SDK)
#define MAX_MUTEX APP_RTOS_MAX_SEMAPHORE_COUNT
#else
#define MAX_MUTEX OSAL_FREERTOS_MAX_SEMAPHOREP_PER_SOC
#endif
#endif
#if defined(A72) || defined(A53) || defined(PC)
    #define MAX_MUTEX TIVX_MUTEX_MAX_OBJECTS
#endif
    tivx_mutex mutex[MAX_MUTEX];

    for (i = 0; i < MAX_MUTEX; i++)
    {
        if((vx_status)VX_SUCCESS != tivxMutexCreate(&mutex[i]))
        {
            break;
        }
    }
    j = i;

    for (i = 0; i < j; i++)
    {
        if((vx_status)VX_SUCCESS != tivxMutexDelete(&mutex[i]))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex at the index %d\n",i);
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetGetTargetKernelInstanceState(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target_kernel_instance_t target_kernel_instance;
    target_kernel_instance.state = VX_TYPE_INVALID;
    vx_enum state;

    if((vx_status)VX_SUCCESS == tivxGetTargetKernelInstanceState(NULL, &state))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }
    else if((vx_status)VX_SUCCESS != tivxGetTargetKernelInstanceState(&target_kernel_instance, &state))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: target kernel instance\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestTargetIsTargetKernelInstanceReplicated(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_target_kernel_instance_t kernel_instance;

    kernel_instance.is_kernel_instance_replicated = vx_true_e;

    if ((vx_bool)vx_true_e != tivxIsTargetKernelInstanceReplicated(&kernel_instance))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: kernel instance replicated\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        kernel_instance.is_kernel_instance_replicated = vx_false_e;

        if ((vx_bool)vx_false_e != tivxIsTargetKernelInstanceReplicated(&kernel_instance))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for ARG: kernel instance not replicated\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestGetObjElement(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc = NULL;
    tivx_obj_desc_object_array_t *obj_desc_object_array = NULL;

    obj_desc = (tivx_obj_desc_t *)ownObjDescAlloc((vx_enum)test_obj_desc->type, NULL);
    if(NULL == obj_desc)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to allocate memory\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        obj_desc_object_array = (tivx_obj_desc_object_array_t *)obj_desc;
        obj_desc_object_array->num_items = 1;
        obj_desc->type = (uint16_t)INVALID_ARG;
        obj_desc->scope_obj_desc_id = (uint16_t)TIVX_OBJ_DESC_INVALID;

        if (obj_desc != tivxGetObjDescElement(obj_desc, (uint16_t)INVALID_ARG))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result for the ARG: obj_desc->scope_obj_desc_id\n");
            status = (vx_status)VX_FAILURE;
        }

        if((vx_status)VX_SUCCESS != ownObjDescFree((tivx_obj_desc_t**)&obj_desc))
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid result: Failed to deallocate memory\n");
            status = (vx_status)VX_FAILURE;
        }
    }
    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}


/*To hit uncovered regions in /psdk_j7/rtos/tivx_queue.c*/
#if defined(R5F) || defined(C7X_FAMILY) || defined(C66)
static vx_status tivxTestQueuePut(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_queue que[10];
    tivx_queue *test_queue=&que[0];
    uintptr_t mem[10];
    uintptr_t *queue_memory = &mem[0];
    uint32_t max_elements = 1;
    tivx_event block_wr_test;

    if(VX_SUCCESS != tivxQueueCreate(test_queue,max_elements,queue_memory,TIVX_QUEUE_FLAG_BLOCK_ON_PUT))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxQueueCreate failed\n");
        status = (vx_status)VX_FAILURE;
    }

    if(VX_SUCCESS != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxQueuePut() exit with FAILURE\n");
        status = (vx_status)VX_FAILURE;
    }
    /*For A72 and A53 the below condition for tivxQueuePut create a deadlock due to pthread_cond_wait() */
    block_wr_test = test_queue->block_wr;
    test_queue->block_wr =NULL;
    if(VX_FAILURE != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"blocking on que put is not disabled\n");
        status = (vx_status)VX_FAILURE;
    }

    test_queue->block_wr =block_wr_test;
    if(VX_SUCCESS != tivxQueueDelete(test_queue))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue Delete failed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestQueueGet(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_queue que[10];
    tivx_queue *test_queue=&que[0];
    uintptr_t mem[10];
    uintptr_t *queue_memory = &mem[0];
    uint32_t max_elements = 1;
    uintptr_t data;
    tivx_event block_rd_test;

    if(VX_SUCCESS != tivxQueueCreate(test_queue,max_elements,queue_memory,TIVX_QUEUE_FLAG_BLOCK_ON_GET))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxQueueCreate failed\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_SUCCESS != tivxQueuePut(test_queue,10,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxEventPost() failed in tivxQueuePut() \n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_SUCCESS != tivxQueueGet(test_queue,&data,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxQueueGet() exit with FAILURE\n");
        status = (vx_status)VX_FAILURE;
    }
    /*For A72 and A53 the below condition for tivxQueueGet create a deadlock due to pthread_cond_wait() */
    block_rd_test = test_queue->block_rd;
    test_queue->block_rd =NULL;
    if(VX_FAILURE!= tivxQueueGet(test_queue,&data,1))
    {
        VX_PRINT(VX_ZONE_ERROR,"blocking on que Get is not disabled\n");
        status = (vx_status)VX_FAILURE;
    }
    test_queue->block_rd =block_rd_test;
    if(VX_SUCCESS != tivxQueueDelete(test_queue))
    {
        VX_PRINT(VX_ZONE_ERROR,"Queue Delete failed\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}
#endif

/*Testcase to fail Queue memory allocation*/
static vx_status tivxTestQueueCreate(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
#if defined(MCU_PLUS_SDK)
    #define MAX_QUEUE APP_RTOS_MAX_SEMAPHORE_COUNT
/*Below macro is used as tivxQueueCreate() depends on tivxEventCreate() which calls appRtosSemaphoreCreate*/
#else
    #define MAX_QUEUE OSAL_FREERTOS_MAX_SEMAPHOREP_PER_SOC
#endif
#endif
#if defined(A72) || defined(A53) || defined(PC)
    #define MAX_QUEUE TIVX_QUEUE_MAX_OBJECTS
#endif
    tivx_queue test_queue[MAX_QUEUE];
    uintptr_t test_qu_mem[MAX_QUEUE];
    tivx_queue *queue_t = &test_queue[0];
    uintptr_t *queue_mem_t=&test_qu_mem[0];
    uint32_t i,j;

    for(i = 0; i < MAX_QUEUE; i++)
    {
        if(VX_SUCCESS != tivxQueueCreate(queue_t, 8, queue_mem_t, TIVX_QUEUE_FLAG_BLOCK_ON_PUT))
        {
            break;
        }
        queue_t++;
    }
    j=i;

    queue_t = &test_queue[0];
    for(i=0; i<j;i++)
    {
        if(VX_SUCCESS !=tivxQueueDelete(queue_t))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to delete Queue at the index %d\n",i);
            status = (vx_status)VX_FAILURE;
            break;
        }
        queue_t++;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestEnableLatch(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxEnableL1DandL2CacheWb();

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestMemResetScratchHeap(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum mheap_region[] = {TIVX_MEM_INTERNAL_L2,
                               TIVX_MEM_EXTERNAL_SCRATCH,TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE};
    uint8_t i, max_region;
    max_region = sizeof(mheap_region)/sizeof(mheap_region[0]);
    for(i=0; i < max_region; i++)
    {
        if(vx_true_e == tivxMemRegionQuery(mheap_region[i]))
        {
            if(VX_SUCCESS != tivxMemResetScratchHeap(mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is not supported tivxMemResetScratchHeap\n",i);
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            if(VX_FAILURE != tivxMemResetScratchHeap(mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is enabled for tivxMemResetScratchHeap\n",i);
                status = (vx_status)VX_FAILURE;
            }
        }
    }
    /*To hit else (negative) region of tivxMemResetScratchHeap*/
    if(VX_FAILURE != tivxMemResetScratchHeap(TIVX_MEM_EXTERNAL))
    {
        VX_PRINT(VX_ZONE_ERROR,"TIVX_MEM_EXTERNAL is supported for tivxMemResetScratchHeap\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxTestMemoryHeapRegions(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum mheap_region[] = {TIVX_MEM_EXTERNAL,TIVX_MEM_INTERNAL_L3, TIVX_MEM_INTERNAL_L2, TIVX_MEM_INTERNAL_L1,
                               TIVX_MEM_EXTERNAL_SCRATCH,TIVX_MEM_EXTERNAL_PERSISTENT_NON_CACHEABLE,
                               TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE,TIVX_MEM_EXTERNAL_CACHEABLE_WT,INVALID_MEM_REGION};

    tivx_shared_mem_ptr_t tsmp[1];
    tivx_shared_mem_ptr_t *mem_ptr = &tsmp[0];
    uint32_t size = 8U, i, max_region;
    void *ptr;
    tivx_mem_stats stat[1];
    tivx_mem_stats *stats = &stat[0];
    max_region = sizeof(mheap_region)/sizeof(mheap_region[0]);

    for(i=0; i < max_region; i++)
    {
        mem_ptr->mem_heap_region= mheap_region[i];

        tivxMemStats(stats, mheap_region[i]);

        if(vx_true_e == tivxMemRegionQuery(mheap_region[i]))
        {
            if(VX_SUCCESS != tivxMemBufferAlloc(mem_ptr, size, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemBufferAlloc\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(0 == tivxMemHost2SharedPtr(mem_ptr->host_ptr, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemHost2SharedPtr\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(0 == tivxMemShared2PhysPtr(mem_ptr->shared_ptr, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemShared2PhysPtr\n",i);
                status = (vx_status)VX_FAILURE;
            }
            ptr= tivxMemAlloc(size, mheap_region[i]);
            if(ptr == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemAlloc\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(VX_SUCCESS !=tivxMemFree(ptr,size, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemFree\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(VX_SUCCESS != tivxMemBufferFree(mem_ptr, size))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemBufferFree\n",i);
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            if(VX_SUCCESS == tivxMemBufferAlloc(mem_ptr, size, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is enabled for tivxMemBufferAlloc\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(0 != tivxMemHost2SharedPtr(0, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemHost2SharedPtr\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(0 != tivxMemShared2PhysPtr(0, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is disabled for tivxMemShared2PhysPtr\n",i);
                status = (vx_status)VX_FAILURE;
            }
            ptr= tivxMemAlloc(size, mheap_region[i]);
            if(ptr != NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is enabled for tivxMemAlloc\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(VX_FAILURE !=tivxMemFree(ptr,size, mheap_region[i]))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is enabled for tivxMemFree\n",i);
                status = (vx_status)VX_FAILURE;
            }
            if(VX_FAILURE != tivxMemBufferFree(mem_ptr, size))
            {
                VX_PRINT(VX_ZONE_ERROR,"mheap_region[%d] is enabled for tivxMemBufferFree\n",i);
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestMemBufferMap(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *host_ptr_t = NULL;

    if(VX_FAILURE !=tivxMemBufferMap(host_ptr_t,1, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemBufferMap not failed with 'host pointer = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*To cover negative conditions of tivxMemTranslateVirtAddr and tivxMemTranslateFd*/
static vx_status tivxNegativeTestMemTranslateFdVirtPhyAddr(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *phyAddr;
    void *virtAddr ;
    uint64_t dmaBufFd;
    uint32_t size;
    size   = 1024;

    if(VX_FAILURE != tivxMemTranslateVirtAddr(NULL, &dmaBufFd, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemTranslateVirtAddr not failed with 'phyAddr = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxMemTranslateVirtAddr(&virtAddr , NULL, &phyAddr))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemTranslateVirtAddr not failed with 'fd = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxMemTranslateVirtAddr(NULL, &dmaBufFd, &phyAddr))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemTranslateVirtAddr not failed with 'virtAddr = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxMemTranslateFd(dmaBufFd, size, &virtAddr, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemTranslateFd not failed with 'phyAddr = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxMemTranslateFd(dmaBufFd, size, NULL, &phyAddr))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemTranslateFd not failed with 'virtAddr = NULL '\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestMemBufferAllocFree(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if(VX_FAILURE != tivxMemBufferAlloc(NULL, 0,(vx_enum)TIVX_MEM_EXTERNAL))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemBufferAlloc not failed with mem_ptr = NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    if(VX_FAILURE != tivxMemBufferFree(NULL, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemBufferFree not failed with mem_ptr = NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestMemBufferUnmap(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if(VX_FAILURE != tivxMemBufferUnmap(NULL, 8,(vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY))
    {
        VX_PRINT(VX_ZONE_ERROR,"tivxMemBufferUnmap not failed with mem_ptr = NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxNegativeTestMemoryStats(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_mem_stats *stats = NULL;

    tivxMemStats(stats, TIVX_MEM_EXTERNAL);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

/*To fail tivxMemTranslateFd() inside tivxMemCompareFd()*/
static vx_status tivxNegativeTestMemCompareFd(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /*value of dmabufFd1 and dmabufFd1 are -1 */
    tivxMemCompareFd(-1u,-1u,8,8);

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}

static vx_status tivxBranchTestTargetObjDescQueue(uint8_t id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint16_t id_t = (vx_enum)TIVX_OBJ_DESC_INVALID -1;
    uint16_t *obj_id = &id_t;

    if((vx_status)VX_FAILURE != ownObjDescQueueRelease(obj_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_id' \n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueEnqueue((vx_enum)TIVX_OBJ_DESC_INVALID,0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueGetCount((vx_enum)TIVX_OBJ_DESC_INVALID, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueDequeue((vx_enum)TIVX_OBJ_DESC_INVALID,obj_id))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueAddBlockedNode((vx_enum)TIVX_OBJ_DESC_INVALID, 0))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }
    if((vx_status)VX_FAILURE != ownObjDescQueueExtractBlockedNodes((vx_enum)TIVX_OBJ_DESC_INVALID, NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid result returned for invalid 'obj_desc_q_id' \n");
        status = (vx_status)VX_FAILURE;
    }

    snprintf(arrOfFuncs[id].funcName, MAX_LENGTH, "%s",__func__);

    return status;
}


FuncInfo arrOfFuncs[] = {
    {tivxTestTargetTaskBoundary, "",VX_SUCCESS},
    {tivxTestTargetObjDescCmpMemset, "",VX_SUCCESS},
    {tivxTestTargetDebugZone, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstance, "",VX_SUCCESS},
    {tivxTestTargetKernelInstance, "",VX_SUCCESS},
    {tivxNegativeTestTargetGetObjDescElement,"",VX_SUCCESS},
    {tivxTestTargetQueryNumTargetKernel, "",VX_SUCCESS},
    {tivxNegativeTestAddTargetKernelInternal,"",VX_SUCCESS},
    {tivxNegativeTestRemoveTargetKernel, "",VX_SUCCESS},
    {tivxTestTargetObjDescAllocFree,"",VX_SUCCESS},
    {tivxTestTargetObjDescQueueCreateRelease, "",VX_SUCCESS},
    {tivxNegativeTestTargetObjDescQueueReleaseGetCount, "",VX_SUCCESS},
    {tivxTestTargetObjDescQueueGetCount, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstanceFree, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstanceGetIndex, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelCreate, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelControl, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelExecute, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelDelete, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelGet, "",VX_SUCCESS},
    {tivxNegativeTestTargetObjDescQueueEnqueue, "", VX_SUCCESS},
    {tivxNegativeTestTargetObjDescQueueExtractBlockedNodes, "",VX_SUCCESS},
    {tivxNegativeTestTargetObjDescQueueAddBlockedNode, "",VX_SUCCESS},
    {tivxNegativeTestTargetObjDescQueueDequeue, "", VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstanceAlloc, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstanceAllocate,"",VX_SUCCESS},
    {tivxBranchTestTargetKernelInstanceAlloc,"",VX_SUCCESS},
    {tivxBranchTestTargetKernelInstanceAllocate,"",VX_SUCCESS},
    {tivxBranchTestTargetKernelInstanceGet,"",VX_SUCCESS},
    {tivxBranchTestTargetKernel, "",VX_SUCCESS},
    {tivxBranchTestTargetKernelFunc, "",VX_SUCCESS},
    {tivxObjDescStrncmp, "", VX_SUCCESS},
    {tivxNegativeTestObjDescSend, "", VX_SUCCESS},
    {tivxNegativeTestObjDescFree,"",VX_SUCCESS},
    {tivxNegativeTestObjDescIsValidType,"",VX_SUCCESS},
    {tivxTestGetObjDescList,"",VX_SUCCESS},
    {tivxTestDescStrncpy,"",VX_SUCCESS},
    {tivxTestDescStrncmpDelim,"",VX_SUCCESS},
    {tivxTestObjDescSend,"",VX_SUCCESS},
    {tivxTestGetObjDescElement,"",VX_SUCCESS},

    #ifndef PC
    {tivxNegativeTestTargetGetHandleAndDelete,"",VX_SUCCESS},
    {tivxTestTargetTriggerNode,"",VX_SUCCESS},
    {tivxNegativeTestTargetQueueObjDesc,"",VX_SUCCESS},
    {tivxNegativeTestObjDescAlloc,"",VX_SUCCESS},
    {tivxNegativeTestTargetGetHandle,"",VX_SUCCESS},
    #endif

    {tivxNegativeTestTargetNodeDescAcquireAllParameter, "", VX_SUCCESS},
    {tivxBranchTestTargetNodeDescAcquireParameter, "",VX_SUCCESS},
    {tivxBranchTestTargetNodeDescAcquireParam,"",VX_SUCCESS},
    {tivxBranchTestTargetNodeDescReleaseAllParameter,"",VX_SUCCESS},
    {tivxBranchTargetNodeDescReleaseParameter,"",VX_SUCCESS},
    {tivxBranchTargetNodeDescReleaseParam,"",VX_SUCCESS},
    {tivxNegativeTestObjDescAllocAndDescQueueCreate,"",VX_SUCCESS},

    #if defined(A72) || defined(A53)
    {tivxTestTargetPlatformGetEnv, "",VX_SUCCESS},
    {tivxTestAppIpcGetIpcCpuId, "",VX_SUCCESS},
    {tivxNegativeTestAppIpcGetAppCpuId, "",VX_SUCCESS},
    {tivxTestAppIpcGetCpuName, "",VX_SUCCESS},
    #endif
    #ifndef PC
    {tivxNegativeTestTargetPlatformDeleteTargetId, "", VX_SUCCESS},
    {tivxNegativeTestTargetPlatformCreateTargetId, "", VX_SUCCESS},
    {tivxAppIpcIsCpuEnabled, "", VX_SUCCESS},
    #endif
    {tivxNegativeTestTargetTaskSetDefaultCreateParams, "", VX_SUCCESS},
    {tivxNegativeTestTargetTaskDelete, "", VX_SUCCESS},
    {tivxNegativeTestTargetTaskCreate, "", VX_SUCCESS},
    #if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
    {tivxAppMemPrintMemAllocInfo, "", VX_SUCCESS},
    {tivxAppMemGetNumAllocs, "", VX_SUCCESS},
    {tivxAppMemUnMap, "", VX_SUCCESS},
#if !defined (MCU_PLUS_SDK)
    {tivxAppIpcGetIpcCpuId, "", VX_SUCCESS},
#endif
    {tivxAppIpcGetAppCpuId, "", VX_SUCCESS},
    {tivxAppIpcGetCpuName, "", VX_SUCCESS},
    {tivxNegativeAppIpcHwLockAcquire, "", VX_SUCCESS},
    {tivxNegativeAppIpcHwLockRelease, "", VX_SUCCESS},
    {tivxNegativeAppIpcGetIpcCpuId, "", VX_SUCCESS},
    {tivxNegativeAppIpcGetAppCpuId, "", VX_SUCCESS},
    {tivxNegativeAppIpcGetCpuName, "", VX_SUCCESS},
    {tivxNegativeAppIpcSendNotifyPort, "", VX_SUCCESS},
    {tivxNegativeAppIpcSendNotify, "", VX_SUCCESS},
    {tivxNegativeAppMemAlloc, "", VX_SUCCESS},
    {tivxNegativeAppMemResetScratchHeap, "", VX_SUCCESS},
    {tivxNegativeAppMemFree, "", VX_SUCCESS},
    {tivxNegativeAppMemStats, "", VX_SUCCESS},
    {tivxNegativeAppRtosSemaphoreParamsInit, "", VX_SUCCESS},
    {tivxNegativeAppRtosSemaphoreDelete, "", VX_SUCCESS},
    {tivxNegativeAppRtosSemaphorePost, "", VX_SUCCESS},
    {tivxNegativeAppRtosTaskParamsInit, "", VX_SUCCESS},
    {tivxNegativeAppRtosTaskDelete, "", VX_SUCCESS},
    {tivxNegativeappRtosSemaphorePend, "", VX_SUCCESS},
    {tivxAppRtosSemaphoreReset, "", VX_SUCCESS},
    {tivxNegativeappRtosTaskCreate, "", VX_SUCCESS},
    {tivxAppRtosTaskSleep, "", VX_SUCCESS},
    {tivxAppRtosTaskYield, "", VX_SUCCESS},
    #endif
    {tivxNegativeTestTargetEventCreate, "",VX_SUCCESS},
    {tivxNegativeTestTargetMutex, "",VX_SUCCESS},
    {tivxTestTargetPlatformGetShmSize, "",VX_SUCCESS},
    #ifndef PC
    {tivxTestTargetEnabled, "",VX_SUCCESS},
    #endif
    {tivxNegativeTestTargetEvent, "",VX_SUCCESS},
    {tivxTestTargetEventClear, "",VX_SUCCESS},
    {tivxTestTargetEventWait, "",VX_SUCCESS},
    #ifndef PC
    {tivxTestTargetPlatformSetHostTargetId, "",VX_SUCCESS},
    #endif
    {tivxNegativeTestTargetPlatformRtos, "",VX_SUCCESS},
    {tivxTestTargetPlatformSystemLock, "",VX_SUCCESS},
    {tivxNegativeTestTargetInitHost, "",VX_SUCCESS},
    #if defined(C7X_FAMILY) || defined(R5F) || defined(C66)
    {tivxNegativeAppRtosSemaphoreCreate, "",VX_SUCCESS},
    {tivxNegativeTestMemStats, "", VX_SUCCESS},
    #endif
    #ifndef PC
    {tivxNegativeTestTargetIpcSendMsg, "",VX_SUCCESS},
    #endif
    {tivxNegativeTaskMain, "",VX_SUCCESS},
    #if defined(R5F) || defined(C7X_FAMILY) || defined(C66)
    {tivxTestQueueCreateDelete, "", VX_SUCCESS},
    {tivxTestQueuePutGetDelete, "", VX_SUCCESS},
    #endif
    #ifndef PC
    #if defined(LINUX)
    {tivxNegativeTaskAppIpcGetHostPortId, "", VX_SUCCESS},
    #endif
    {tivxNegativeTaskAppIpcGetIpcCpuId, "", VX_SUCCESS},
    {tivxNegativeTaskAppIpcGetAppCpuId, "", VX_SUCCESS},
    {tivxNegativeTaskAppIpcGetCpuName, "", VX_SUCCESS},
    {tivxNegativeTaskAappIpcHwLockAcquire, "", VX_SUCCESS},
    {tivxNegativeTaskAppIpcHwLockRelease, "", VX_SUCCESS},
    {tivxNegativeTaskAppIpcSendNotifyPort, "", VX_SUCCESS},
    {tivxNegativeTaskAppIpcSendNotify, "", VX_SUCCESS},
    #endif
    {tivxNegativeTestMutexMaxOut, "",VX_SUCCESS},
    {tivxTestTargetGetTargetKernelInstanceState, "",VX_SUCCESS},
    {tivxTestTargetIsTargetKernelInstanceReplicated, "",VX_SUCCESS},
    {tivxTestGetObjElement, "",VX_SUCCESS},
    #if defined(R5F) || defined(C7X_FAMILY) || defined(C66)
    {tivxTestQueuePut, "", VX_SUCCESS},
    {tivxTestQueueGet, "", VX_SUCCESS},
    #endif
    {tivxTestQueueCreate, "",VX_SUCCESS},
    {tivxTestEnableLatch, "", VX_SUCCESS},
    {tivxTestMemResetScratchHeap, "",VX_SUCCESS},
    {tivxTestMemoryHeapRegions, "", VX_SUCCESS},
    {tivxNegativeTestMemBufferMap, "", VX_SUCCESS},
    {tivxNegativeTestMemTranslateFdVirtPhyAddr, "",VX_SUCCESS},
    {tivxNegativeTestMemBufferAllocFree,"",VX_SUCCESS},
    {tivxNegativeTestMemBufferUnmap,"",VX_SUCCESS},
    {tivxNegativeTestMemoryStats,"",VX_SUCCESS},
    {tivxNegativeTestMemCompareFd,"",VX_SUCCESS},
    {tivxBranchTestTargetObjDescQueue,"",VX_SUCCESS}
};
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
        uint8_t i = 0;
        uint8_t pcount = 0;
        uint8_t fcount = 0;
        vx_status status1 = (vx_status)VX_SUCCESS;
        uint32_t size= sizeof(arrOfFuncs)/sizeof(arrOfFuncs[0]);
        test_kernel = kernel;
        test_obj_desc=obj_desc[0];
        tivx_set_debug_zone(VX_ZONE_INFO);

        if((vx_status)VX_SUCCESS == status)
        {
            for(i=0;i<size;i++)
            {
                status1 = arrOfFuncs[i].funcPtr(i);
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"[ !FAILED! ] TARGET TESTCASE: %s\n",arrOfFuncs[i].funcName);
                    arrOfFuncs[i].status=status1;
                    status = status1;
                    fcount++;
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO,"[ PASSED ] TARGET TESTCASE: %s\n",arrOfFuncs[i].funcName);
                    pcount++;
                }
            }
        }
        VX_PRINT(VX_ZONE_INFO,"------------------REMOTE-CORE TESTCASES SUMMARY-------------------------\n");
        VX_PRINT(VX_ZONE_INFO,"[ ALL DONE ] %d test(s) from 1 test case(s) ran\n",i);
        VX_PRINT(VX_ZONE_INFO,"[ PASSED   ] %d test(s)\n",pcount);
        if(fcount>0)
        {
            i=0;
            VX_PRINT(VX_ZONE_INFO,"[ FAILED   ] %d test(s), listed below:\n",fcount);
            while(i<size)
            {
                if(arrOfFuncs[i].status!= VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO,"[ FAILED   ] %s\n",arrOfFuncs[i].funcName);
                }
                i++;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_INFO,"[ FAILED   ] %d test(s)\n",fcount);
        }
        VX_PRINT(VX_ZONE_INFO,"------------------------------------------------------------------------\n");
        tivx_clr_debug_zone(VX_ZONE_INFO);
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
    #if defined(SOC_J721E)
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    #endif

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)))
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
    #if defined(SOC_J721E)
    else if (self_cpu == TIVX_CPU_ID_DSP_C7_1)
    {
        strncpy(target_name, TIVX_TARGET_DSP_C7_1, TIVX_TARGET_MAX_NAME);
        vx_test_target_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_TEST_TARGET_NAME,
                            target_name,
                            tivxTestTargetProcess,
                            tivxTestTargetCreate,
                            tivxTestTargetDelete,
                            tivxTestTargetControl,
                            NULL);
    }
    #endif
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


