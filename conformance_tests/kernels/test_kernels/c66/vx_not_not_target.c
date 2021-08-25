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
#include "TI/tivx_test_kernels.h"
#include "VX/vx.h"
#include "tivx_test_kernels_kernels.h"
#include "tivx_kernel_not_not.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "ti/vxlib/vxlib.h"

static tivx_target_kernel vx_not_not_target_kernel = NULL;

static vx_status VX_CALLBACK tivxNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxNotNotProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_image_t *output_desc;

    if ( (num_params != TIVX_KERNEL_NOT_NOT_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_INPUT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NOT_NOT_OUTPUT_IDX];

    }

    if(VX_SUCCESS == status)
    {

        void *input_target_ptr;
        void *output_target_ptr;

        input_target_ptr = tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(input_target_ptr,
           input_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_READ_ONLY));

        output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr,
           output_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
           VX_WRITE_ONLY));



        {
            VXLIB_bufParams2D_t vxlib_input;
            uint8_t *input_addr = NULL;
            VXLIB_bufParams2D_t vxlib_output;
            uint8_t *output_addr = NULL;

            tivxInitBufParams(input_desc, &vxlib_input);
            tivxSetPointerLocation(input_desc, &input_target_ptr, &input_addr);

            tivxInitBufParams(output_desc, &vxlib_output);
            tivxSetPointerLocation(output_desc, &output_target_ptr, &output_addr);

            status = VXLIB_not_i8u_o8u(input_target_ptr, &vxlib_input, output_target_ptr, &vxlib_output);
            status = VXLIB_not_i8u_o8u(output_target_ptr, &vxlib_output, output_target_ptr, &vxlib_output);

        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(input_target_ptr,
           input_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr,
           output_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY));



    }

    return status;
}

static vx_status VX_CALLBACK tivxNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */

    return status;
}

static vx_status VX_CALLBACK tivxNotNotDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */

    return status;
}


void tivxAddTargetKernelNotNot(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_not_not_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_NOT_NOT_NAME,
                            target_name,
                            tivxNotNotProcess,
                            tivxNotNotCreate,
                            tivxNotNotDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelNotNot(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_not_not_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_not_not_target_kernel = NULL;
    }
}


