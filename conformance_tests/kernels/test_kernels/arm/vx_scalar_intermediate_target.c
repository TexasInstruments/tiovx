/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include "TI/tivx_capture.h"
#include "VX/vx.h"
#include "tivx_capture_kernels.h"
#include "tivx_kernel_scalar_intermediate.h"
#include "TI/tivx_target_kernel.h"
#include <TI/tivx_task.h>
#include "tivx_kernels_target_utils.h"

#define TEST_MODE_CREATE_FAILURE_VAL  10
static uint8_t process_failure = 0u;
static uint8_t delete_failure = 0u;

static vx_status VX_CALLBACK tivxScalarIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxScalarIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxScalarIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxScalarIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxScalarIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_scalar_t *in_desc;
    tivx_obj_desc_scalar_t *out_desc;

    if ( (num_params != TIVX_KERNEL_SCALAR_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_IN_IDX];
        out_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_OUT_IDX];
    }

    if(VX_SUCCESS == status)
    {
        vx_uint8 in_value;

        in_value = in_desc->data.u08;

        if (process_failure == 1)
        {
            status = VX_FAILURE;
        }

        out_desc->data.u08 = in_value;

    }

    return status;
}

static vx_status VX_CALLBACK tivxScalarIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_scalar_t *in_desc;

    if ( (num_params != TIVX_KERNEL_SCALAR_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_SCALAR_INTERMEDIATE_IN_IDX];
    }

    if(VX_SUCCESS == status)
    {
        vx_uint8 in_value;

        process_failure = 0;
        delete_failure = 0;

        in_value = in_desc->data.u08;

        if (in_value == TEST_MODE_CREATE_FAILURE_VAL)
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxScalarIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if (delete_failure == 1)
    {
        status = VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxScalarIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    switch (node_cmd_id)
    {
        case TIVX_SCALAR_INTERMEDIATE_REPLICATE_QUERY:
        {
            if (NULL != obj_desc[0])
            {
                void *target_ptr;
                tivx_obj_desc_user_data_object_t *usr_data_obj;
                tivx_scalar_intermediate_control_t *replicate_prms = NULL;

                usr_data_obj = (tivx_obj_desc_user_data_object_t *)obj_desc[0U];

                target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                if (sizeof(tivx_scalar_intermediate_control_t) ==
                        usr_data_obj->mem_size)
                {
                    replicate_prms = (tivx_scalar_intermediate_control_t *)target_ptr;

                    replicate_prms->is_target_kernel_replicated = tivxIsTargetKernelInstanceReplicated(kernel);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        }
        case TIVX_SCALAR_INTERMEDIATE_PROCESS_FAIL:
        {
            process_failure = 1;
            break;
        }
        case TIVX_SCALAR_INTERMEDIATE_DELETE_FAIL:
        {
            delete_failure = 1;
            break;
        }
        default:
            break;
    }

    return status;
}

void tivxAddTargetKernelScalarIntermediate(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameC7x(target_name)))
    {
        tivxAddTargetKernelByName(
                            TIVX_KERNEL_SCALAR_INTERMEDIATE_NAME,
                            target_name,
                            tivxScalarIntermediateProcess,
                            tivxScalarIntermediateCreate,
                            tivxScalarIntermediateDelete,
                            tivxScalarIntermediateControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelScalarIntermediate(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameC7x(target_name)))
    {
        tivxRemoveTargetKernelByName(TIVX_KERNEL_SCALAR_INTERMEDIATE_NAME,
            target_name);
    }
}


