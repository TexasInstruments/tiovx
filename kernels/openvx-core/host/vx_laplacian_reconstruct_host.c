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
#include <tivx_kernel_laplacian_reconstruct.h>
#include <TI/tivx_target_kernel.h>
#include <math.h>

static vx_kernel vx_laplacian_reconstruct_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructValidate(vx_node node,
            const vx_reference parameters[],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image src_img, out_img;
    vx_pyramid pmd;
    vx_uint32 w, h, i;
    vx_uint32 input_w, input_h;
    vx_uint32 p_w, p_h;
    vx_df_image fmt, o_fmt, p_fmt;
    vx_float32 scale;
    vx_border_t border;
    vx_size levels;

    for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
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
        src_img = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];
        pmd = (vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];
        out_img = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(src_img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));

        status |= vxQueryImage(src_img, VX_IMAGE_WIDTH, &input_w, sizeof(input_w));
        status |= vxQueryImage(src_img, VX_IMAGE_HEIGHT, &input_h, sizeof(input_h));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR,"Due to a bug in the OpenVX 1.1 Spec, the U8 data type is the only data type supported for Laplacian Reconstruct images\n");
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_SCALE, &scale,
            sizeof(scale));
        if (VX_SUCCESS == status)
        {
            if (scale!= VX_SCALE_PYRAMID_HALF)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryPyramid(pmd, VX_PYRAMID_WIDTH, &p_w, sizeof(p_w));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt));
        status |= vxQueryPyramid(pmd, VX_PYRAMID_LEVELS, &levels, sizeof(levels));

        /* Check for validity of data format */
        if (VX_DF_IMAGE_S16 != p_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)pmd)))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(out_img, VX_IMAGE_FORMAT, &o_fmt, sizeof(o_fmt));
        status |= vxQueryImage(out_img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(out_img, VX_IMAGE_HEIGHT, &h, sizeof(h));

        /* Check for frame sizes */
        if ((w != p_w) || (h != p_h))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (o_fmt != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR,"Due to a bug in the OpenVX 1.1 Spec, the U8 data type is the only data type supported for Laplacian Reconstruct images\n");
        }

        if (VX_SUCCESS == status)
        {
            for (i = 0; i < levels; i++)
            {
                w = (vx_uint32)ceilf(w * scale);
                h = (vx_uint32)ceilf(h * scale);
            }

            /* Check for frame sizes */
            if ((w != input_w) || (h != input_h))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }

    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));
        if (VX_SUCCESS == status)
        {
            if ((border.mode != VX_BORDER_UNDEFINED) &&
                (border.mode != VX_BORDER_REPLICATE))
            {
                status = VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only undefined and replicate border mode is supported for laplacian reconstruct\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if (NULL != metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX])
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_WIDTH, &p_w, sizeof(p_w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_HEIGHT, &p_h, sizeof(p_h));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX],
                VX_IMAGE_FORMAT, &o_fmt, sizeof(o_fmt));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;
    vx_uint32 i;
    vx_image img;
    vx_size levels;
    vx_pyramid pyr;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        pyr = (vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];

        status |= vxQueryPyramid(pyr, VX_PYRAMID_LEVELS, &levels, sizeof(levels));

    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);
        img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX], 0);
        prms.in_img[0] = img;
        prms.in_img[1] = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];
        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX];

        prms.num_input_images = 2;
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
        for (i = 1; i < levels-1; i++)
        {
            img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX], i);
            prms.in_img[0] = img;

            prms.num_input_images = 1;
            prms.num_output_images = 0;

            prms.top_pad = 0;
            prms.bot_pad = 0;
            prms.left_pad = 0;
            prms.right_pad = 0;

            prms.border_mode = VX_BORDER_UNDEFINED;

            status = tivxKernelConfigValidRect(&prms);
            status |= vxReleaseImage(&img);
        }
    }

    /*if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);
        img = vxGetPyramidLevel((vx_pyramid)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX], (levels-1));
        prms.in_img[0] = img;
        prms.in_img[1] = (vx_image)parameters[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];

        prms.num_input_images = 2;
        prms.num_output_images = 0;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;

        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
        status |= vxReleaseImage(&img);
    }*/

    return status;
}

vx_status tivxAddKernelLaplacianReconstruct(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.laplacian_reconstruct",
                            VX_KERNEL_LAPLACIAN_RECONSTRUCT,
                            NULL,
                            TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS,
                            tivxAddKernelLaplacianReconstructValidate,
                            tivxAddKernelLaplacianReconstructInitialize,
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
                VX_TYPE_PYRAMID,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
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
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
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

    vx_laplacian_reconstruct_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelLaplacianReconstruct(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_laplacian_reconstruct_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_laplacian_reconstruct_kernel = NULL;

    return status;
}





