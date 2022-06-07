/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <VX/vx.h>
#include <TI/tivx_event.h>
#include <TI/tivx_task.h>
#include <TI/tivx_test_kernels.h>
#include <tivx_test_kernels_kernels.h>
#include <tivx_kernel_cmd_timeout_test.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

static tivx_target_kernel vx_cmd_timeout_test_target_kernel = NULL;

static vx_status VX_CALLBACK tivxCmdTimeoutTestProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCmdTimeoutTestCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCmdTimeoutTestDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCmdTimeoutTestControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxCmdTimeoutTestProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *cfgDesc;
    tivx_obj_desc_scalar_t *in_desc;
    tivx_obj_desc_scalar_t *out_desc;

    if ((num_params != TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void                       *cfgTgtPtr;
        tivx_cmd_timeout_params_t  *cfgParams;

        cfgDesc = (tivx_obj_desc_user_data_object_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX];

        in_desc = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX];

        out_desc = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX];

        cfgTgtPtr = tivxMemShared2TargetPtr(&cfgDesc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(cfgTgtPtr,
                         cfgDesc->mem_size,
                         (vx_enum)VX_MEMORY_TYPE_HOST,
                         (vx_enum)VX_READ_ONLY));

        cfgParams = (tivx_cmd_timeout_params_t *)cfgTgtPtr;

        /* Copy input scalar to the output. */
        out_desc->data.u08 = in_desc->data.u08;

        if ((cfgParams->processCmdTimeout != 0) &&
            (cfgParams->processCmdTimeout != TIVX_EVENT_TIMEOUT_WAIT_FOREVER))
        {
            tivxTaskWaitMsecs(cfgParams->processCmdTimeout);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(cfgTgtPtr,
                           cfgDesc->mem_size,
                           (vx_enum)VX_MEMORY_TYPE_HOST,
                           (vx_enum)VX_READ_ONLY));
    }

    return status;
}

static vx_status VX_CALLBACK tivxCmdTimeoutTestCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *cfgDesc;

    if ((num_params != TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void                       *cfgTgtPtr;
        tivx_cmd_timeout_params_t  *cfgParams;

        cfgDesc = (tivx_obj_desc_user_data_object_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX];

        cfgTgtPtr = tivxMemShared2TargetPtr(&cfgDesc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(cfgTgtPtr,
                         cfgDesc->mem_size,
                         (vx_enum)VX_MEMORY_TYPE_HOST,
                         (vx_enum)VX_READ_ONLY));

        cfgParams = (tivx_cmd_timeout_params_t *)cfgTgtPtr;

        if ((cfgParams->createCmdTimeout != 0) &&
            (cfgParams->createCmdTimeout != TIVX_EVENT_TIMEOUT_WAIT_FOREVER))
        {
            tivxTaskWaitMsecs(cfgParams->createCmdTimeout);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(cfgTgtPtr,
                           cfgDesc->mem_size,
                           (vx_enum)VX_MEMORY_TYPE_HOST,
                           (vx_enum)VX_READ_ONLY));
    }

    return status;
}

static vx_status VX_CALLBACK tivxCmdTimeoutTestDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *cfgDesc;

    if ((num_params != TIVX_KERNEL_CMD_TIMEOUT_TEST_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_OUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void                       *cfgTgtPtr;
        tivx_cmd_timeout_params_t  *cfgParams;

        cfgDesc = (tivx_obj_desc_user_data_object_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX];

        cfgTgtPtr = tivxMemShared2TargetPtr(&cfgDesc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(cfgTgtPtr,
                         cfgDesc->mem_size,
                         (vx_enum)VX_MEMORY_TYPE_HOST,
                         (vx_enum)VX_READ_ONLY));

        cfgParams = (tivx_cmd_timeout_params_t *)cfgTgtPtr;

        if ((cfgParams->deleteCmdTimeout != 0) &&
            (cfgParams->deleteCmdTimeout != TIVX_EVENT_TIMEOUT_WAIT_FOREVER))
        {
            tivxTaskWaitMsecs(cfgParams->deleteCmdTimeout);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(cfgTgtPtr,
                           cfgDesc->mem_size,
                           (vx_enum)VX_MEMORY_TYPE_HOST,
                           (vx_enum)VX_READ_ONLY));
    }

    return status;
}

static vx_status VX_CALLBACK tivxCmdTimeoutTestControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *cfgDesc;

    if ((num_params != 1) || (NULL == obj_desc[0])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void                       *cfgTgtPtr;
        tivx_cmd_timeout_params_t  *cfgParams;

        cfgDesc = (tivx_obj_desc_user_data_object_t *)
            obj_desc[TIVX_KERNEL_CMD_TIMEOUT_TEST_CONFIGURATION_IDX];

        cfgTgtPtr = tivxMemShared2TargetPtr(&cfgDesc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(cfgTgtPtr,
                         cfgDesc->mem_size,
                         (vx_enum)VX_MEMORY_TYPE_HOST,
                         (vx_enum)VX_READ_ONLY));

        cfgParams = (tivx_cmd_timeout_params_t *)cfgTgtPtr;

        if ((cfgParams->controlCmdTimeout != 0) &&
            (cfgParams->controlCmdTimeout != TIVX_EVENT_TIMEOUT_WAIT_FOREVER))
        {
            tivxTaskWaitMsecs(cfgParams->controlCmdTimeout);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(cfgTgtPtr,
                           cfgDesc->mem_size,
                           (vx_enum)VX_MEMORY_TYPE_HOST,
                           (vx_enum)VX_READ_ONLY));
    }

    return status;
}

void tivxAddTargetKernelCmdTimeoutTest(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_A72_0 )
    {
        strncpy(target_name, TIVX_TARGET_A72_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #if defined(SOC_AM62A)
    else
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_MCU1_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #else
    else
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0 )
    {
        strncpy(target_name, TIVX_TARGET_MCU2_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_1 )
    {
        strncpy(target_name, TIVX_TARGET_MCU2_1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_cmd_timeout_test_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_CMD_TIMEOUT_TEST_NAME,
                            target_name,
                            tivxCmdTimeoutTestProcess,
                            tivxCmdTimeoutTestCreate,
                            tivxCmdTimeoutTestDelete,
                            tivxCmdTimeoutTestControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelCmdTimeoutTest(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_cmd_timeout_test_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_cmd_timeout_test_target_kernel = NULL;
    }
}


