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
/* Maximum length of testcase function name */
#define MAX_LENGTH 64

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

typedef struct {
    vx_status (*funcPtr)(uint8_t);
    char funcName[MAX_LENGTH];
    vx_status status;
} FuncInfo;

FuncInfo arrOfFuncs[];

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

    uint32_t *targetId = (uint32_t *)tivxMemAlloc(sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    vx_border_t *border_mode = (vx_border_t *)tivxMemAlloc(sizeof(vx_border_t),TIVX_MEM_EXTERNAL);

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

    tivxMemFree((void *)targetId,sizeof(uint32_t), TIVX_MEM_EXTERNAL);
    tivxMemFree((void *)border_mode,sizeof(vx_border_t), TIVX_MEM_EXTERNAL);

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

FuncInfo arrOfFuncs[] = {
    {tivxTestTargetTaskBoundary, "",VX_SUCCESS},
    {tivxTestTargetObjDescCmpMemset, "",VX_SUCCESS},
    {tivxTestTargetDebugZone, "",VX_SUCCESS},
    {tivxNegativeTestTargetKernelInstance, "",VX_SUCCESS},
    {tivxTestTargetKernelInstance,"",VX_SUCCESS}
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
    uint8_t i = 0;
    uint8_t pcount = 0;
    uint8_t fcount = 0;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t size= sizeof(arrOfFuncs)/sizeof(arrOfFuncs[0]);
    test_kernel = kernel;
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


