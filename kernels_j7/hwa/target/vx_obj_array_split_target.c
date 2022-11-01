/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
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
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_obj_array_split.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

#define OBJ_ARR_SPLIT_MAX_KERNELS 4U

static tivx_target_kernel vx_obj_array_split_target_kernel[OBJ_ARR_SPLIT_MAX_KERNELS] = {NULL};

static void swapObjArray(
    tivx_obj_desc_object_array_t *in_desc,
    tivx_obj_desc_t *in_elem_desc[],
    tivx_obj_desc_object_array_t *out_desc,
    tivx_obj_desc_t *out_elem_desc[],
    uint32_t index
    )
{
    uint32_t i, j = 0;

    for (i = index; i < index+out_desc->num_items; i++)
    {
        in_desc->obj_desc_id[i]  = out_elem_desc[j]->obj_desc_id;
        out_desc->obj_desc_id[j] = in_elem_desc[i]->obj_desc_id;
        j++;
    }
}

static vx_status VX_CALLBACK tivxObjArraySplitProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxObjArraySplitCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxObjArraySplitDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxObjArraySplitControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxObjArraySplitProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_object_array_t *in_desc;
    tivx_obj_desc_t *in_elem_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    vx_uint32 index = 0;
    tivx_obj_desc_object_array_t *out0_desc;
    tivx_obj_desc_t *out0_elem_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_object_array_t *out1_desc;
    tivx_obj_desc_t *out1_elem_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_object_array_t *out2_desc;
    tivx_obj_desc_t *out2_elem_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    tivx_obj_desc_object_array_t *out3_desc;
    tivx_obj_desc_t *out3_elem_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];

    if ( (num_params != TIVX_KERNEL_OBJ_ARRAY_SPLIT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        in_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX];
        out0_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX];
        out1_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX];
        out2_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT2_IDX];
        out3_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT3_IDX];
    }

    if((vx_status)VX_SUCCESS == status)
    {
        tivxGetObjDescList(in_desc->obj_desc_id, (tivx_obj_desc_t**)in_elem_desc, in_desc->num_items);

        tivxGetObjDescList(out0_desc->obj_desc_id, (tivx_obj_desc_t**)out0_elem_desc, out0_desc->num_items);

        tivxGetObjDescList(out1_desc->obj_desc_id, (tivx_obj_desc_t**)out1_elem_desc, out1_desc->num_items);

        if( out2_desc != NULL)
        {
            tivxGetObjDescList(out2_desc->obj_desc_id, (tivx_obj_desc_t**)out2_elem_desc, out2_desc->num_items);
        }

        if( out3_desc != NULL)
        {
            tivxGetObjDescList(out3_desc->obj_desc_id, (tivx_obj_desc_t**)out3_elem_desc, out3_desc->num_items);
        }

        swapObjArray(in_desc, in_elem_desc, out0_desc, out0_elem_desc, index);

        index += out0_desc->num_items;

        swapObjArray(in_desc, in_elem_desc, out1_desc, out1_elem_desc, index);

        index += out1_desc->num_items;

        if( out2_desc != NULL)
        {
            swapObjArray(in_desc, in_elem_desc, out2_desc, out2_elem_desc, index);

            index += out2_desc->num_items;
        }

        if( out3_desc != NULL)
        {
            swapObjArray(in_desc, in_elem_desc, out3_desc, out3_elem_desc, index);

            index += out3_desc->num_items;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxObjArraySplitCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_OBJ_ARRAY_SPLIT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_OBJ_ARRAY_SPLIT_OUT1_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxObjArraySplitDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxObjArraySplitControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelObjArraySplit(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[OBJ_ARR_SPLIT_MAX_KERNELS][TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_A72_0 )
    {
        strncpy(target_name[0], TIVX_TARGET_A72_0, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[1], TIVX_TARGET_A72_1, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[2], TIVX_TARGET_A72_2, TIVX_TARGET_MAX_NAME);
        strncpy(target_name[3], TIVX_TARGET_A72_3, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        int i;

        for (i = 0; i < OBJ_ARR_SPLIT_MAX_KERNELS; i++)
        {
            vx_obj_array_split_target_kernel[i] = tivxAddTargetKernelByName(
                                TIVX_KERNEL_OBJ_ARRAY_SPLIT_NAME,
                                target_name[i],
                                tivxObjArraySplitProcess,
                                tivxObjArraySplitCreate,
                                tivxObjArraySplitDelete,
                                tivxObjArraySplitControl,
                                NULL);
        }
    }
}

void tivxRemoveTargetKernelObjArraySplit(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int i;

    for (i = 0; i < OBJ_ARR_SPLIT_MAX_KERNELS; i++)
    {
        status = tivxRemoveTargetKernel(vx_obj_array_split_target_kernel[i]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_obj_array_split_target_kernel[i] = NULL;
        }
    }
}


