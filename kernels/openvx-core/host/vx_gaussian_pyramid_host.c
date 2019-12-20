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

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_gaussian_pyramid.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

#if !defined(J6_VSDK)
#include <TI/j7.h>
#endif

static vx_kernel vx_gaussian_pyramid_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_pyramid gaussian = NULL;
    vx_uint32 gaussian_w;
    vx_uint32 gaussian_h;
    vx_df_image gaussian_fmt;
    vx_float32 gaussian_scale;
    vx_size gaussian_levels;

    vx_border_t border;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input = (vx_image)parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_INPUT_IDX];
        gaussian = (vx_pyramid)parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_FORMAT, &gaussian_fmt, sizeof(gaussian_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_WIDTH, &gaussian_w, sizeof(gaussian_w)));
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_HEIGHT, &gaussian_h, sizeof(gaussian_h)));
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_SCALE, &gaussian_scale, sizeof(gaussian_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_LEVELS, &gaussian_levels, sizeof(gaussian_levels)));

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)gaussian);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8!= input_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != gaussian_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'gaussian' should be a pyramid of type:\n VX_DF_IMAGE_U8 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (input_w != gaussian_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input' and 'gaussian' should have the same value for VX_IMAGE_WIDTH \n");
            }

            if (input_h != gaussian_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'input' and 'gaussian' should have the same value for VX_IMAGE_HEIGHT \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((VX_SCALE_PYRAMID_HALF != gaussian_scale) &&
            (VX_SCALE_PYRAMID_ORB != gaussian_scale))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'gaussian' should be a pyramid of scale:\n VX_SCALE_PYRAMID_HALF or VX_SCALE_PYRAMID_ORB \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED != border.mode)
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for gaussian pyramid \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], (vx_enum)VX_PYRAMID_WIDTH, &input_w, sizeof(input_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], (vx_enum)VX_PYRAMID_HEIGHT, &input_h, sizeof(input_h));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], (vx_enum)VX_PYRAMID_FORMAT, &input_fmt, sizeof(input_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], (vx_enum)VX_PYRAMID_LEVELS, &gaussian_levels, sizeof(gaussian_levels));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], (vx_enum)VX_PYRAMID_SCALE, &gaussian_scale, sizeof(gaussian_scale));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    vx_pyramid gaussian = NULL;
    vx_float32 gaussian_scale;
    vx_size gaussian_levels;

    vx_image img;
    vx_image in_img;
    vx_image out_img;

    vx_uint32 i;

    if (num_params != TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid number of parameters\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        gaussian = (vx_pyramid)parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_SCALE, &gaussian_scale, sizeof(gaussian_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(gaussian, (vx_enum)VX_PYRAMID_LEVELS, &gaussian_levels, sizeof(gaussian_levels)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_INPUT_IDX];

        img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX], 0);

        prms.out_img[0U] = img;

        prms.num_input_images = 1U;
        prms.num_output_images = 1U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
        tivxCheckStatus(&status, vxReleaseImage(&img));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (VX_SCALE_PYRAMID_HALF == gaussian_scale)
        {
            for (i = 1U; i < gaussian_levels; i++)
            {
                in_img = vxGetPyramidLevel(gaussian, i - 1U);

                out_img = vxGetPyramidLevel(gaussian, i);

                prms.out_img[0U] = out_img;

                prms.num_input_images = 1U;
                prms.num_output_images = 1U;

                prms.top_pad = 1U;
                prms.bot_pad = 1U;
                prms.left_pad = 1U;
                prms.right_pad = 1U;

                prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

                tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));

                tivxCheckStatus(&status, vxReleaseImage(&out_img));
                tivxCheckStatus(&status, vxReleaseImage(&in_img));
            }
        }
        else
        {
            for (i = 1U; i < gaussian_levels; i++)
            {
                in_img = vxGetPyramidLevel(gaussian, i - 1U);

                out_img = vxGetPyramidLevel(gaussian, i);

                prms.in_img[0U] = in_img;
                prms.out_img[0U] = out_img;

                prms.num_input_images = 1U;
                prms.num_output_images = 1U;

                prms.top_pad = 2U;
                prms.bot_pad = 2U;
                prms.left_pad = 2U;
                prms.right_pad = 2U;

                prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

                tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));

                tivxCheckStatus(&status, vxReleaseImage(&out_img));
                tivxCheckStatus(&status, vxReleaseImage(&in_img));
            }
        }
    }

    return status;
}

vx_status tivxAddKernelGaussianPyramid(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    "org.khronos.openvx.gaussian_pyramid",
                    (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                    NULL,
                    TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS,
                    tivxAddKernelGaussianPyramidValidate,
                    tivxAddKernelGaussianPyramidInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
#if !defined(J6_VSDK)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2);
#endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_gaussian_pyramid_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelGaussianPyramid(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_gaussian_pyramid_kernel;

    status = vxRemoveKernel(kernel);
    vx_gaussian_pyramid_kernel = NULL;

    return status;
}


