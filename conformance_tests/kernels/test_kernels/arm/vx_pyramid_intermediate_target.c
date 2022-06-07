/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_pyramid_intermediate.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

static tivx_target_kernel vx_pyramid_intermediate_target_kernel = NULL;

static vx_status VX_CALLBACK tivxPyramidIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxPyramidIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxPyramidIntermediateProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_pyramid_t *input_desc;
    tivx_obj_desc_image_t *img_input_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    vx_uint32 i;
    tivx_obj_desc_pyramid_t *output_desc;
    tivx_obj_desc_image_t *img_output_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint8_t *input_addr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS], *output_addr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    if ( (num_params != TIVX_KERNEL_PYRAMID_INTERMEDIATE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_PYRAMID_INTERMEDIATE_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_PYRAMID_INTERMEDIATE_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        input_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_PYRAMID_INTERMEDIATE_INPUT_IDX];
        output_desc = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_PYRAMID_INTERMEDIATE_OUTPUT_IDX];

    }

    if(VX_SUCCESS == status)
    {

        void *input_target_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS] = {NULL};
        void *output_target_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS] = {NULL};

        tivxGetObjDescList(input_desc->obj_desc_id, (tivx_obj_desc_t**)img_input_desc, input_desc->num_levels);
        for(i=0; i<input_desc->num_levels; i++)
        {
            input_target_ptr[i] = tivxMemShared2TargetPtr(&img_input_desc[i]->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(input_target_ptr[i],
               img_input_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
               VX_READ_ONLY));
        }

        tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t**)img_output_desc, output_desc->num_levels);
        for(i=0; i<output_desc->num_levels; i++)
        {
            output_target_ptr[i] = tivxMemShared2TargetPtr(&img_output_desc[i]->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr[i],
               img_output_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
               VX_WRITE_ONLY));
        }



        for(i=0; i<input_desc->num_levels; i++)
        {
            tivxSetPointerLocation(img_input_desc[i], &input_target_ptr[i], &input_addr[i]);
            tivxSetPointerLocation(img_output_desc[i], &output_target_ptr[i], &output_addr[i]);

            memcpy(output_addr[i], input_addr[i], img_output_desc[i]->imagepatch_addr[0].stride_y*img_output_desc[i]->imagepatch_addr[0].dim_y);
        }
        for(i=0; i<input_desc->num_levels; i++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(input_target_ptr[i],
               img_input_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY));
        }

        for(i=0; i<output_desc->num_levels; i++)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr[i],
               img_output_desc[i]->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY));
        }



    }

    return status;
}

static vx_status VX_CALLBACK tivxPyramidIntermediateCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */

    return status;
}

static vx_status VX_CALLBACK tivxPyramidIntermediateDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */

    return status;
}

static vx_status VX_CALLBACK tivxPyramidIntermediateControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands */
    /*                   the user can call to modify the processing of the kernel at run-time) > */

    return status;
}

void tivxAddTargetKernelPyramidIntermediate(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    #if defined(SOC_AM62A)
    if ((self_cpu == TIVX_CPU_ID_MCU1_0))
    {
        strncpy(target_name, TIVX_TARGET_MCU1_0, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #else
    if ( (self_cpu == TIVX_CPU_ID_MCU2_0) ||
          (self_cpu == TIVX_CPU_ID_MCU2_1))
    {
        if (self_cpu == TIVX_CPU_ID_MCU2_0)
        {
            strncpy(target_name, TIVX_TARGET_MCU2_0, TIVX_TARGET_MAX_NAME);
            status = (vx_status)VX_SUCCESS;
        }
        else if (self_cpu == TIVX_CPU_ID_MCU2_1)
        {
            strncpy(target_name, TIVX_TARGET_MCU2_1, TIVX_TARGET_MAX_NAME);
            status = (vx_status)VX_SUCCESS;
        }
    }
    #endif
    else
    {
        status = VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_pyramid_intermediate_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_PYRAMID_INTERMEDIATE_NAME,
                            target_name,
                            tivxPyramidIntermediateProcess,
                            tivxPyramidIntermediateCreate,
                            tivxPyramidIntermediateDelete,
                            tivxPyramidIntermediateControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelPyramidIntermediate(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_pyramid_intermediate_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_pyramid_intermediate_target_kernel = NULL;
    }
}


