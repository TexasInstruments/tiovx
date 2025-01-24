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
#include "tivx_capture_kernels.h"
#include "tivx_kernel_image_intermediate.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_image_intermediate_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelImageIntermediateValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelImageIntermediateInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
static vx_status VX_CALLBACK tivxAddKernelImageIntermediateDeinitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelImageIntermediate(vx_context context);
vx_status tivxRemoveKernelImageIntermediate(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelImageIntermediateValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image in = NULL;
    vx_uint32 in_w;
    vx_uint32 in_h;
    vx_df_image in_fmt;

    if ( (num != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    else
    {
        in = (vx_image)parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(in, (vx_enum)VX_IMAGE_WIDTH, &in_w, sizeof(in_w)));
        tivxCheckStatus(&status, vxQueryImage(in, (vx_enum)VX_IMAGE_HEIGHT, &in_h, sizeof(in_h)));
        tivxCheckStatus(&status, vxQueryImage(in, (vx_enum)VX_IMAGE_FORMAT, &in_fmt, sizeof(in_fmt)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX], (vx_enum)VX_IMAGE_FORMAT, &in_fmt, sizeof(in_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX], (vx_enum)VX_IMAGE_WIDTH, &in_w, sizeof(in_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX], (vx_enum)VX_IMAGE_HEIGHT, &in_h, sizeof(in_h));
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelImageIntermediateInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_image in, out;

    if ( (num_params != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    else
    {
        in  = (vx_image)parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
        out = (vx_image)parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX];
        vx_imagepatch_addressing_t addr;
        vx_uint8 *pdata = 0;
        vx_rectangle_t rect = {0, 0, 1, 1};
        vx_map_id map_id;

        /* Note: test of mapping an image pointer, should not fail */
        status = vxMapImagePatch(in, &rect, 0, &map_id, &addr, (void **)&pdata,
                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (status == VX_SUCCESS)
        {
            vxUnmapImagePatch(in, map_id);
            status = vxMapImagePatch(out, &rect, 0, &map_id, &addr, (void **)&pdata,
                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);;

            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map of output parameter failed!\n");
            }
            else
            {
                vxUnmapImagePatch(out, map_id);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Map of input parameter failed!\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelImageIntermediateDeinitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_image in, out;

    if ( !((num_params != TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX]))
    )
    {
        in  = (vx_image)parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_IN_IDX];
        out = (vx_image)parameters[TIVX_KERNEL_IMAGE_INTERMEDIATE_OUT_IDX];
        vx_imagepatch_addressing_t addr;
        vx_uint8 *pdata = 0;
        vx_rectangle_t rect = {0, 0, 1, 1};
        vx_map_id map_id;

        /* Note: test of mapping an image pointer, should not fail */
        status = vxMapImagePatch(in, &rect, 0, &map_id, &addr, (void **)&pdata,
                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (status == VX_SUCCESS)
        {
            vxUnmapImagePatch(in, map_id);
            status = vxMapImagePatch(out, &rect, 0, &map_id, &addr, (void **)&pdata,
                                    VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);;

            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "Map of output parameter failed!\n");
            }
            else
            {
                vxUnmapImagePatch(out, map_id);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Map of input parameter failed!\n");
        }
    }

    return status;
}

vx_status tivxAddKernelImageIntermediate(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_IMAGE_INTERMEDIATE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_IMAGE_INTERMEDIATE_MAX_PARAMS,
                    tivxAddKernelImageIntermediateValidate,
                    tivxAddKernelImageIntermediateInitialize,
                    tivxAddKernelImageIntermediateDeinitialize);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetMcu(kernel);
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_0);
            #if defined(SOC_J721E)
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1);
            #endif
        }
        if (status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_image_intermediate_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelImageIntermediate(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_image_intermediate_kernel;

    status = vxRemoveKernel(kernel);
    vx_image_intermediate_kernel = NULL;

    return status;
}


