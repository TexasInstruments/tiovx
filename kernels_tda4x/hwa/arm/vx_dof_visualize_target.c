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

#ifdef BUILD_HWA_DMPAC_DOF

#include "TI/tivx.h"
#include "TI/tda4x.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dof_visualize.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "LibDenseOpticalFlow.h"

static tivx_target_kernel vx_dof_visualize_target_kernel = NULL;

static vx_status VX_CALLBACK tivxDofVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDofVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDofVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDofVisualizeControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxDofVisualizeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *flow_vector_desc;
    tivx_obj_desc_image_t *flow_vector_rgb_desc;
    tivx_obj_desc_image_t *confidence_image_desc;

    if ( num_params != TIVX_KERNEL_DOF_VISUALIZE_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_RGB_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_IMAGE_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {

        flow_vector_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_IDX];
        flow_vector_rgb_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DOF_VISUALIZE_FLOW_VECTOR_RGB_IDX];
        confidence_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DOF_VISUALIZE_CONFIDENCE_IMAGE_IDX];

        flow_vector_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          flow_vector_desc->mem_ptr[0].shared_ptr, flow_vector_desc->mem_ptr[0].mem_type);
        flow_vector_rgb_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          flow_vector_rgb_desc->mem_ptr[0].shared_ptr, flow_vector_rgb_desc->mem_ptr[0].mem_type);
        confidence_image_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          confidence_image_desc->mem_ptr[0].shared_ptr, confidence_image_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(flow_vector_desc->mem_ptr[0].target_ptr,
           flow_vector_desc->mem_size[0], flow_vector_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(flow_vector_rgb_desc->mem_ptr[0].target_ptr,
           flow_vector_rgb_desc->mem_size[0], flow_vector_rgb_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        tivxMemBufferMap(confidence_image_desc->mem_ptr[0].target_ptr,
           confidence_image_desc->mem_size[0], confidence_image_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);


        /* call kernel processing function */
        {
            int flow_vector_size[2];

            flow_vector_size[0] = flow_vector_desc->height;
            flow_vector_size[1] = flow_vector_desc->imagepatch_addr[0].stride_y/sizeof(uint32_t);

            visualizeFlowAndConfidance(
                flow_vector_desc->mem_ptr[0].target_ptr,
                flow_vector_size,
                flow_vector_rgb_desc->mem_ptr[0].target_ptr,
                confidence_image_desc->mem_ptr[0].target_ptr,
                flow_vector_desc->width,
                flow_vector_rgb_desc->imagepatch_addr[0].stride_y,
                confidence_image_desc->imagepatch_addr[0].stride_y
             );
        }

        /* kernel processing function complete */

        tivxMemBufferUnmap(flow_vector_desc->mem_ptr[0].target_ptr,
           flow_vector_desc->mem_size[0], flow_vector_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(flow_vector_rgb_desc->mem_ptr[0].target_ptr,
           flow_vector_rgb_desc->mem_size[0], flow_vector_rgb_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        tivxMemBufferUnmap(confidence_image_desc->mem_ptr[0].target_ptr,
           confidence_image_desc->mem_size[0], confidence_image_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);


    }

    return status;
}

static vx_status VX_CALLBACK tivxDofVisualizeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;


    return status;
}

static vx_status VX_CALLBACK tivxDofVisualizeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxDofVisualizeControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;


    return status;
}

void tivxAddTargetKernelDofVisualize()
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_IPU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_IPU1_0, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_dof_visualize_target_kernel = tivxAddTargetKernel(
                            TIVX_KERNEL_DOF_VISUALIZE,
                            target_name,
                            tivxDofVisualizeProcess,
                            tivxDofVisualizeCreate,
                            tivxDofVisualizeDelete,
                            tivxDofVisualizeControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelDofVisualize()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dof_visualize_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dof_visualize_target_kernel = NULL;
    }
}

#endif

