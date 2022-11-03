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
#include <tivx_kernel_laplacian_reconstruct.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"
#include <math.h>

static vx_kernel vx_laplacian_reconstruct_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_pyramid laplacian = NULL;
    vx_uint32 laplacian_w;
    vx_uint32 laplacian_h;
    vx_df_image laplacian_fmt;
    vx_float32 laplacian_scale;
    vx_size laplacian_levels;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_image output = NULL;
    vx_uint32 output_w;
    vx_uint32 output_h;
    vx_df_image output_fmt;

    vx_border_t border;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    vx_uint32 w;
    vx_uint32 h;
    vx_uint32 i;

    if ( (num != TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_LAPLACIAN_IDX])
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        laplacian = (vx_pyramid)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_LAPLACIAN_IDX];
        input = (vx_image)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_INPUT_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_FORMAT, &laplacian_fmt, sizeof(laplacian_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_WIDTH, &laplacian_w, sizeof(laplacian_w)));
        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_HEIGHT, &laplacian_h, sizeof(laplacian_h)));
        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_SCALE, &laplacian_scale, sizeof(laplacian_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_LEVELS, &laplacian_levels, sizeof(laplacian_levels)));

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)output);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_S16 != laplacian_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'laplacian' should be a pyramid of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_U8 != input_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != output_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        w = output_w;
        h = output_h;

        for (i = 0U; i < laplacian_levels; i++)
        {
            w = (vx_uint32)ceilf((vx_float32)w * laplacian_scale);
            h = (vx_uint32)ceilf((vx_float32)h * laplacian_scale);
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (laplacian_w != output_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'laplacian' and 'output' should have the same value for 'width' \n");
            }

            if (laplacian_h != output_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'laplacian' and 'output' should have the same value for 'height' \n");
            }
        }

        if (w != input_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Invalid value of 'width' for 'input' with given value 'levels' for 'laplacian' and value 'width' for 'output' \n");
        }

        if (h != input_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Invalid value of 'height' for 'input' with given value 'levels' for 'laplacian' and value 'height' for 'output' \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (VX_SCALE_PYRAMID_HALF != laplacian_scale)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'laplacian' should have 'scale' of type:\n VX_SCALE_PYRAMID_HALF \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_enum)VX_BORDER_UNDEFINED != border.mode) && ((vx_enum)VX_BORDER_REPLICATE != border.mode))
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined and replicate border mode is supported for laplacian reconstruct \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        output_fmt = (vx_df_image)VX_DF_IMAGE_U8;

        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX], (vx_enum)VX_IMAGE_WIDTH, &laplacian_w, sizeof(laplacian_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX], (vx_enum)VX_IMAGE_HEIGHT, &laplacian_h, sizeof(laplacian_h));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX], (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelLaplacianReconstructInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    vx_pyramid laplacian;
    vx_size laplacian_levels;

    vx_image img;
    vx_uint32 i;

    if ( (num_params != TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_LAPLACIAN_IDX])
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        laplacian = (vx_pyramid)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_LAPLACIAN_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(laplacian, (vx_enum)VX_PYRAMID_LEVELS, &laplacian_levels, sizeof(laplacian_levels)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);
        img = vxGetPyramidLevel(laplacian, 0U);
        prms.in_img[0U] = img;
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_INPUT_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_OUTPUT_IDX];

        prms.num_input_images = 2U;
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
        for (i = 1U; i < (laplacian_levels - 1U); i++)
        {
            img = vxGetPyramidLevel(laplacian, i);
            prms.in_img[0U] = img;

            prms.num_input_images = 1U;
            prms.num_output_images = 0U;

            prms.top_pad = 0U;
            prms.bot_pad = 0U;
            prms.left_pad = 0U;
            prms.right_pad = 0U;

            prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
        tivxCheckStatus(&status, vxReleaseImage(&img));
        }
    }

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
                (vx_enum)VX_KERNEL_LAPLACIAN_RECONSTRUCT,
                NULL,
                TIVX_KERNEL_LAPLACIAN_RECONSTRUCT_MAX_PARAMS,
                tivxAddKernelLaplacianReconstructValidate,
                tivxAddKernelLaplacianReconstructInitialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);

    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
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
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
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
    vx_laplacian_reconstruct_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelLaplacianReconstruct(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_laplacian_reconstruct_kernel;

    status = vxRemoveKernel(kernel);
    vx_laplacian_reconstruct_kernel = NULL;

    return status;
}


