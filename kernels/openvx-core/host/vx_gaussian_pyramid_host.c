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



#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_gaussian_pyramid.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_gausspmd_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_pyramid pmd;
    vx_uint32 w, h, i;
    vx_uint32 p_w, p_h;
    vx_size num_levels;
    vx_df_image fmt, p_fmt;
    vx_float32 scale;
    vx_border_t border;

    for (i = 0U; i < TIVX_KERNEL_G_PYD_MAX_PARAMS; i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        img = (vx_image)parameters[TIVX_KERNEL_G_PYD_IN_IMG_IDX];
        pmd = (vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));

        status |= vxQueryImage(img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_SCALE, &scale,
            sizeof(scale));
        if (VX_SUCCESS == status)
        {
            if ((scale!= VX_SCALE_PYRAMID_HALF) &&
                (scale != VX_SCALE_PYRAMID_ORB))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_LEVELS, &num_levels,
            sizeof(num_levels));
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)pmd)))
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_WIDTH, &p_w, sizeof(p_w));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt));

        /* Check for frame sizes */
        if ((w != p_w) || (h != p_h))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (fmt != p_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if (border.mode != VX_BORDER_UNDEFINED)
            {
                status = VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for gaussian pyramid\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if (NULL != metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX],
                VX_PYRAMID_WIDTH, &w, sizeof(w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX],
                VX_PYRAMID_HEIGHT, &h, sizeof(h));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX],
                VX_PYRAMID_FORMAT, &fmt, sizeof(fmt));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX],
                VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_G_PYD_OUT_PYT_IDX],
                VX_PYRAMID_SCALE, &scale, sizeof(scale));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelGaussianPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    tivxKernelValidRectParams prms;
    vx_pyramid pmd;
    vx_size num_levels;
    vx_image img, in_img, out_img;
    vx_float32 scale;

    if (num_params != TIVX_KERNEL_G_PYD_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        pmd = (vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX];

        status |= vxQueryPyramid(pmd, VX_PYRAMID_LEVELS, &num_levels,
            sizeof(num_levels));

        status |= vxQueryPyramid(pmd, VX_PYRAMID_SCALE, &scale,
            sizeof(scale));
    }

    if (VX_SUCCESS == status)
    {
        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_G_PYD_IN_IMG_IDX];
        img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX], 0);

        prms.out_img[0] = img;

        prms.num_input_images = 1;
        prms.num_output_images = 1;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;

        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
        status |= vxReleaseImage(&img);
    }

    if (VX_SUCCESS == status)
    {
        if (scale == VX_SCALE_PYRAMID_HALF)
        {
            for (i = 1; i < num_levels; i++)
            {
                in_img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX], i-1);

                out_img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX], i);

                prms.out_img[0] = out_img;

                prms.num_input_images = 1;
                prms.num_output_images = 1;

                prms.top_pad = 1;
                prms.bot_pad = 1;
                prms.left_pad = 1;
                prms.right_pad = 1;

                prms.border_mode = VX_BORDER_UNDEFINED;

                status |= tivxKernelConfigValidRect(&prms);

                status |= vxReleaseImage(&out_img);
                status |= vxReleaseImage(&in_img);
            }
        }
        else
        {
            for (i = 1; i < num_levels; i++)
            {
                in_img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX], i-1);

                out_img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_G_PYD_OUT_PYT_IDX], i);

                prms.in_img[0] = in_img;
                prms.out_img[0] = out_img;

                prms.num_input_images = 1;
                prms.num_output_images = 1;

                prms.top_pad = 2;
                prms.bot_pad = 2;
                prms.left_pad = 2;
                prms.right_pad = 2;

                prms.border_mode = VX_BORDER_UNDEFINED;

                status |= tivxKernelConfigValidRect(&prms);

                status |= vxReleaseImage(&out_img);
                status |= vxReleaseImage(&in_img);
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

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.gaussian_pyramid",
                            VX_KERNEL_GAUSSIAN_PYRAMID,
                            NULL,
                            2,
                            tivxAddKernelGaussianPyramidValidate,
                            tivxAddKernelGaussianPyramidInitialize,
                            NULL);

    status = vxGetStatus((vx_reference)kernel);

    if ( status == VX_SUCCESS)
    {
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_PYRAMID,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
            tivxAddKernelTarget(kernel, TIVX_TARGET_RESV03);
            tivxAddKernelTarget(kernel, TIVX_TARGET_RESV04);
        }

        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if( status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }

    vx_gausspmd_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelGaussianPyramid(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_gausspmd_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_gausspmd_kernel = NULL;

    return status;
}





