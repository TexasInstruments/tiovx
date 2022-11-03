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
#include <tivx_kernel_channel_combine.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_channel_combine_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelChannelCombineValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelChannelCombineInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
static vx_status tivxKernelConfigChannelCombineValidRect(tivxKernelValidRectParams *prms);

static vx_status VX_CALLBACK tivxAddKernelChannelCombineValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image plane0 = NULL;
    vx_uint32 plane0_w;
    vx_uint32 plane0_h;
    vx_df_image plane0_fmt;

    vx_image plane1 = NULL;
    vx_uint32 plane1_w;
    vx_uint32 plane1_h;
    vx_df_image plane1_fmt;

    vx_image plane2 = NULL;
    vx_uint32 plane2_w;
    vx_uint32 plane2_h;
    vx_df_image plane2_fmt;

    vx_image plane3 = NULL;
    vx_uint32 plane3_w;
    vx_uint32 plane3_h;
    vx_df_image plane3_fmt;

    vx_image output = NULL;
    vx_uint32 output_w;
    vx_uint32 output_h;
    vx_df_image output_fmt;

    vx_uint16 in_channel = 2;
    vx_uint16 out_channel;

    if ( (num != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        plane0 = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX];
        plane1 = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX];
        plane2 = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
        plane3 = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(plane0, (vx_enum)VX_IMAGE_WIDTH, &plane0_w, sizeof(plane0_w)));
        tivxCheckStatus(&status, vxQueryImage(plane0, (vx_enum)VX_IMAGE_HEIGHT, &plane0_h, sizeof(plane0_h)));
        tivxCheckStatus(&status, vxQueryImage(plane0, (vx_enum)VX_IMAGE_FORMAT, &plane0_fmt, sizeof(plane0_fmt)));

        tivxCheckStatus(&status, vxQueryImage(plane1, (vx_enum)VX_IMAGE_WIDTH, &plane1_w, sizeof(plane1_w)));
        tivxCheckStatus(&status, vxQueryImage(plane1, (vx_enum)VX_IMAGE_HEIGHT, &plane1_h, sizeof(plane1_h)));
        tivxCheckStatus(&status, vxQueryImage(plane1, (vx_enum)VX_IMAGE_FORMAT, &plane1_fmt, sizeof(plane1_fmt)));

        if (NULL != plane2)
        {
            in_channel++;

            tivxCheckStatus(&status, vxQueryImage(plane2, (vx_enum)VX_IMAGE_WIDTH, &plane2_w, sizeof(plane2_w)));
            tivxCheckStatus(&status, vxQueryImage(plane2, (vx_enum)VX_IMAGE_HEIGHT, &plane2_h, sizeof(plane2_h)));
            tivxCheckStatus(&status, vxQueryImage(plane2, (vx_enum)VX_IMAGE_FORMAT, &plane2_fmt, sizeof(plane2_fmt)));
        }
        else
        {
            plane2_w = 0U;
            plane2_h = 0U;
        }

        if (NULL != plane3)
        {
            in_channel++;

            tivxCheckStatus(&status, vxQueryImage(plane3, (vx_enum)VX_IMAGE_WIDTH, &plane3_w, sizeof(plane3_w)));
            tivxCheckStatus(&status, vxQueryImage(plane3, (vx_enum)VX_IMAGE_HEIGHT, &plane3_h, sizeof(plane3_h)));
            tivxCheckStatus(&status, vxQueryImage(plane3, (vx_enum)VX_IMAGE_FORMAT, &plane3_fmt, sizeof(plane3_fmt)));
        }
        else
        {
            plane3_w = 0U;
            plane3_h = 0U;
        }

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8 != plane0_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'plane0' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_U8 != plane1_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'plane1' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if (NULL != plane2)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != plane2_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'plane2' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }

        if (NULL != plane3)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != plane3_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'plane3' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }

        if (((vx_df_image)VX_DF_IMAGE_RGBX != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_RGB != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV12 != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV21 != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_YUYV != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_UYVY != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_IYUV != output_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_YUV4 != output_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_RGBX or VX_DF_IMAGE_RGB or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_NV21 or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_IYUV or VX_DF_IMAGE_YUV4 \n");
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        out_channel = 0U;

        if ((vx_df_image)VX_DF_IMAGE_RGBX == output_fmt)
        {
            out_channel = 4U;
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGB == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_NV12 == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_NV21 == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_YUYV == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_UYVY == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_IYUV == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_YUV4 == output_fmt))
        {
            out_channel = 3U;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid output format \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if (in_channel != out_channel)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid number of plane channels \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        if ((vx_df_image)VX_DF_IMAGE_RGBX == output_fmt)
        {
            if ((plane1_w != plane0_w) ||
                ((NULL != plane2) && (plane2_w != plane0_w)) ||
                ((NULL != plane3) && (plane3_w != plane0_w)))
            {
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'plane0' and secondary planes should have the same value for VX_IMAGE_WIDTH \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            if ((plane1_h != plane0_h) ||
                ((NULL != plane2) && (plane2_h != plane0_h)) ||
                ((NULL != plane3) && (plane3_h != plane0_h)))
            {
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'plane0' and secondary should have the same value for VX_IMAGE_HEIGHT \n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else if (((vx_df_image)VX_DF_IMAGE_RGB == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_YUV4 == output_fmt))
        {
                if ((plane1_w != plane0_w) ||
                    ((NULL != plane2) && (plane2_w != plane0_w)))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'plane0' and secondary planes should have the same value for VX_IMAGE_WIDTH \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                if ((plane1_h != plane0_h) ||
                    ((NULL != plane2) && (plane2_h != plane0_h)))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'plane0' and secondary should have the same value for VX_IMAGE_HEIGHT \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
        }
        else if (((vx_df_image)VX_DF_IMAGE_NV12 == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_NV21 == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_IYUV == output_fmt))
        {
                if ((plane1_w != (plane0_w / 2U)) ||
                    ((NULL != plane2) && (plane2_w != (plane0_w / 2U))))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Secondary planes should have half the value of 'plane0' for VX_IMAGE_WIDTH \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                if ((plane1_h != (plane0_h / 2U)) ||
                    ((NULL != plane2) && (plane2_h != (plane0_h / 2U))))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Secondary planes should have half the value of 'plane0' for VX_IMAGE_HEIGHT \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
        }
        else if (((vx_df_image)VX_DF_IMAGE_YUYV == output_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_UYVY == output_fmt))
        {
                if ((plane1_w != (plane0_w / 2U)) ||
                    ((NULL != plane2) && (plane2_w != (plane0_w / 2U))))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Secondary planes should have half the value of 'plane0' for VX_IMAGE_WIDTH \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                if ((plane1_h != plane0_h) ||
                    ((NULL != plane2) && (plane2_h != plane0_h)))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'plane0' and secondary planes should have the same value for VX_IMAGE_HEIGHT  \n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid output format \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX], (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX], (vx_enum)VX_IMAGE_WIDTH, &plane0_w, sizeof(plane0_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX], (vx_enum)VX_IMAGE_HEIGHT, &plane0_h, sizeof(plane0_h));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelChannelCombineInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE0_IDX];
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE1_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_OUTPUT_IDX];

        prms.num_input_images = 2U;
        prms.num_output_images = 1U;

        if (NULL != parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX])
        {
            prms.in_img[prms.num_input_images] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
            prms.num_input_images++;
        }

        if (NULL != parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE3_IDX])
        {
            prms.in_img[prms.num_input_images] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_PLANE2_IDX];
            prms.num_input_images++;
        }

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigChannelCombineValidRect(&prms));
    }

    return status;
}

vx_status tivxAddKernelChannelCombine(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.channel_combine",
                (vx_enum)VX_KERNEL_CHANNEL_COMBINE,
                NULL,
                TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS,
                tivxAddKernelChannelCombineValidate,
                tivxAddKernelChannelCombineInitialize,
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
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
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
    vx_channel_combine_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelChannelCombine(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_channel_combine_kernel;

    status = vxRemoveKernel(kernel);
    vx_channel_combine_kernel = NULL;

    return status;
}

static vx_status tivxKernelConfigChannelCombineValidRect(tivxKernelValidRectParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_rectangle_t out_rect, rect;
    vx_uint32 i;
    vx_df_image fmt;

    if (NULL == prms)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if ((prms->num_input_images >
                TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE) ||
            (prms->num_output_images >
                TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE))
        {
                status = (vx_status)VX_FAILURE;
        }

        for (i = 0; i < prms->num_input_images; i ++)
        {
            if (NULL == prms->in_img[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
        for (i = 0; i < prms->num_output_images; i ++)
        {
            if (NULL == prms->out_img[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxQueryImage(prms->out_img[0], (vx_enum)VX_IMAGE_FORMAT, &fmt, sizeof(fmt));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        out_rect.start_y = 0;
        out_rect.start_x = 0;
        out_rect.end_y = 0xFFFFFFFFU;
        out_rect.end_x = 0xFFFFFFFFU;

        for (i = 0; i < prms->num_input_images; i ++)
        {
            status = vxGetValidRegionImage(prms->in_img[i], &rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            else
            {
                if ((i == 0U) ||
                    ((fmt != (vx_df_image)VX_DF_IMAGE_IYUV) && (fmt != (vx_df_image)VX_DF_IMAGE_NV12) &&
                     (fmt != (vx_df_image)VX_DF_IMAGE_NV21) && (fmt != (vx_df_image)VX_DF_IMAGE_YUYV) && (fmt != (vx_df_image)VX_DF_IMAGE_UYVY)))
                {
                    if (rect.start_x > out_rect.start_x)
                    {
                        out_rect.start_x = rect.start_x;
                    }
                    if (rect.start_y > out_rect.start_y)
                    {
                        out_rect.start_y = rect.start_y;
                    }

                    if (rect.end_x < out_rect.end_x)
                    {
                        out_rect.end_x = rect.end_x;
                    }
                    if (rect.end_y < out_rect.end_y)
                    {
                        out_rect.end_y = rect.end_y;
                    }
                }
                else {
                    if ((rect.start_x*2U) > out_rect.start_x)
                    {
                        out_rect.start_x = rect.start_x*2U;
                    }
                    if ((rect.start_y*2U) > out_rect.start_y)
                    {
                        out_rect.start_y = rect.start_y*2U;
                    }

                    if ((rect.end_x*2U) < out_rect.end_x)
                    {
                        out_rect.end_x = rect.end_x*2U;
                    }
                    if ((rect.end_y*2U) < out_rect.end_y)
                    {
                        out_rect.end_y = rect.end_y*2U;
                    }
                }
            }
        }
        for (i = 0; (i < prms->num_output_images) && ((vx_status)VX_SUCCESS == status); i ++)
        {
            status = vxGetValidRegionImage(prms->out_img[i], &rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            else
            {
                if (rect.start_x > out_rect.start_x)
                {
                    out_rect.start_x = rect.start_x;
                }
                if (rect.start_y > out_rect.start_y)
                {
                    out_rect.start_y = rect.start_y;
                }

                if (rect.end_x < out_rect.end_x)
                {
                    out_rect.end_x = rect.end_x;
                }
                if (rect.end_y < out_rect.end_y)
                {
                    out_rect.end_y = rect.end_y;
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED == prms->border_mode)
        {
            out_rect.start_x += prms->left_pad;
            out_rect.start_y += prms->top_pad;
            out_rect.end_x -= prms->right_pad;
            out_rect.end_y -= prms->bot_pad;
        }

        for (i = 0; i < prms->num_output_images; i ++)
        {
            status = vxSetImageValidRectangle(prms->out_img[i], &out_rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
        }

    }
    return (status);
}
