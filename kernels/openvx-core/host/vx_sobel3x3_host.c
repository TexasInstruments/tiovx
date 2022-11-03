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
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_sobel3x3_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelSobel3X3Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelSobel3X3Initialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelSobel3X3Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_image output_x = NULL;
    vx_uint32 output_x_w;
    vx_uint32 output_x_h;
    vx_df_image output_x_fmt;

    vx_image output_y = NULL;
    vx_uint32 output_y_w;
    vx_uint32 output_y_h;
    vx_df_image output_y_fmt;

    vx_border_t border;

    vx_bool is_output_x_virtual = (vx_bool)vx_false_e;
    vx_bool is_output_y_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SOBEL3X3_INPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_INPUT_IDX];
        output_x = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        output_y = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));

        if (NULL != output_x)
        {
            tivxCheckStatus(&status, vxQueryImage(output_x, (vx_enum)VX_IMAGE_WIDTH, &output_x_w, sizeof(output_x_w)));
            tivxCheckStatus(&status, vxQueryImage(output_x, (vx_enum)VX_IMAGE_HEIGHT, &output_x_h, sizeof(output_x_h)));
            tivxCheckStatus(&status, vxQueryImage(output_x, (vx_enum)VX_IMAGE_FORMAT, &output_x_fmt, sizeof(output_x_fmt)));
        }

        if (NULL != output_y)
        {
            tivxCheckStatus(&status, vxQueryImage(output_y, (vx_enum)VX_IMAGE_WIDTH, &output_y_w, sizeof(output_y_w)));
            tivxCheckStatus(&status, vxQueryImage(output_y, (vx_enum)VX_IMAGE_HEIGHT, &output_y_h, sizeof(output_y_h)));
            tivxCheckStatus(&status, vxQueryImage(output_y, (vx_enum)VX_IMAGE_FORMAT, &output_y_fmt, sizeof(output_y_fmt)));
        }

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_output_x_virtual = tivxIsReferenceVirtual((vx_reference)output_x);
        is_output_y_virtual = tivxIsReferenceVirtual((vx_reference)output_y);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8 != input_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if (NULL != output_x)
        {
            if ((vx_bool)vx_false_e == is_output_x_virtual)
            {
                if ((vx_df_image)VX_DF_IMAGE_S16 != output_x_fmt)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'output_x' should be an image of type:\n VX_DF_IMAGE_S16 \n");
                }
            }
        }

        if (NULL != output_y)
        {
            if ((vx_bool)vx_false_e == is_output_y_virtual)
            {
                if ((vx_df_image)VX_DF_IMAGE_S16 != output_y_fmt)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'output_y' should be an image of type:\n VX_DF_IMAGE_S16 \n");
                }
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != output_x)
        {
            if ((vx_bool)vx_false_e == is_output_x_virtual)
            {
                if (output_x_w != input_w)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output_x' and 'input' should have the same value for VX_IMAGE_WIDTH \n");
                }

                if (output_x_h != input_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output_x' and 'input' should have the same value for VX_IMAGE_HEIGHT \n");
                }
            }
        }

        if (NULL != output_y)
        {
            if ((vx_bool)vx_false_e == is_output_y_virtual)
            {
                if (output_y_w != input_w)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output_y' and 'input' should have the same value for VX_IMAGE_WIDTH \n");
                }

                if (output_y_h != input_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output_y' and 'input' should have the same value for VX_IMAGE_HEIGHT \n");
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED != border.mode)
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for sobel \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        output_x_fmt = (vx_df_image)VX_DF_IMAGE_S16;

        if (NULL != output_x)
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX], (vx_enum)VX_IMAGE_FORMAT, &output_x_fmt, sizeof(output_x_fmt));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX], (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX], (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h));
        }

        if (NULL != output_y)
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX], (vx_enum)VX_IMAGE_FORMAT, &output_y_fmt, sizeof(output_y_fmt));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX], (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w));
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX], (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h));
        }
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelSobel3X3Initialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    vx_image output_x = NULL;
    vx_image output_y = NULL;

    if ( (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SOBEL3X3_INPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        output_x = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        output_y = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);


        prms.num_input_images = 1U;
        prms.num_output_images = 0U;

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_SOBEL3X3_INPUT_IDX];

        if (NULL != output_x)
        {
            prms.out_img[prms.num_output_images] = output_x;
            prms.num_output_images++;
        }

        if (NULL != output_y)
        {
            prms.out_img[prms.num_output_images] = output_y;
            prms.num_output_images++;
        }

        prms.top_pad = 1U;
        prms.bot_pad = 1U;
        prms.left_pad = 1U;
        prms.right_pad = 1U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;
        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
    }

    return status;
}

vx_status tivxAddKernelSobel3X3(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.sobel_3x3",
                (vx_enum)VX_KERNEL_SOBEL_3x3,
                NULL,
                TIVX_KERNEL_SOBEL3X3_MAX_PARAMS,
                tivxAddKernelSobel3X3Validate,
                tivxAddKernelSobel3X3Initialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);

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
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
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
    vx_sobel3x3_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSobel3X3(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_sobel3x3_kernel;

    status = vxRemoveKernel(kernel);
    vx_sobel3x3_kernel = NULL;

    return status;
}


