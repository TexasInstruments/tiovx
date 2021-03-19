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
#include "TI/j7.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_video_decoder.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_video_decoder_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVideoDecoderValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVideoDecoderInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelVideoDecoderValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_video_decoder_params_t configuration_value;

    vx_user_data_object input_bitstream = NULL;
    vx_char input_bitstream_name[VX_MAX_REFERENCE_NAME];
    vx_size input_bitstream_size;

    vx_image output_image = NULL;
    vx_uint32 output_image_w;
    vx_uint32 output_image_h;
    vx_df_image output_image_fmt;

    if ( (num != TIVX_KERNEL_VIDEO_DECODER_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX])
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX];
        input_bitstream = (vx_user_data_object)parameters[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX];
        output_image = (vx_image)parameters[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(configuration, 0, sizeof(tivx_video_decoder_params_t), &configuration_value, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryUserDataObject(input_bitstream, (vx_enum)VX_USER_DATA_OBJECT_NAME, &input_bitstream_name, sizeof(input_bitstream_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(input_bitstream, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &input_bitstream_size, sizeof(input_bitstream_size)));

        tivxCheckStatus(&status, vxQueryImage(output_image, (vx_enum)VX_IMAGE_FORMAT, &output_image_fmt, sizeof(output_image_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output_image, (vx_enum)VX_IMAGE_WIDTH, &output_image_w, sizeof(output_image_w)));
        tivxCheckStatus(&status, vxQueryImage(output_image, (vx_enum)VX_IMAGE_HEIGHT, &output_image_h, sizeof(output_image_h)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_video_decoder_params_t)) ||
            (strncmp(configuration_name, "tivx_video_decoder_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_video_decoder_params_t \n");
        }

        if (strncmp(input_bitstream_name, "video_bitstream", sizeof(input_bitstream_name)) != 0)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_bitstream_name' should be a user_data_object named 'video_bitstream' of type:\n uint8_t[1] \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_NV12 != output_image_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_image' should be an image of type:\n VX_DF_IMAGE_NV12 \n");
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (TIVX_BITSTREAM_FORMAT_H264 != configuration_value.bitstream_format)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Decoder param 'bitstream_format' should be:\n TIVX_BITSTREAM_FORMAT_H264 \n");
        }

        if ((output_image_w % 64) != 0)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Decoder param 'output_image' width should be multiple of 64\n");
        }

        if ((output_image_h % 64) != 0)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Decoder param 'output_image' height should be multiple of 64\n");
        }
    }

    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_NV12 == output_image_fmt) && ((output_image_w * output_image_h * 3U / 2U) > input_bitstream_size))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Decoder param 'input stream' should be at least:\n output_width * output_height * 3 / 2 \n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVideoDecoderInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_VIDEO_DECODER_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_INPUT_BITSTREAM_IDX])
        || (NULL == parameters[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_VIDEO_DECODER_OUTPUT_IMAGE_IDX];

        prms.num_input_images = 0;
        prms.num_output_images = 1;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelVideoDecoder(vx_context context)
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
                    TIVX_KERNEL_VIDEO_DECODER_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VIDEO_DECODER_MAX_PARAMS,
                    tivxAddKernelVideoDecoderValidate,
                    tivxAddKernelVideoDecoderInitialize,
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
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_VDEC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VDEC2);
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
    vx_video_decoder_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVideoDecoder(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_video_decoder_kernel;

    status = vxRemoveKernel(kernel);
    vx_video_decoder_kernel = NULL;

    return status;
}

void tivx_video_decoder_params_init(tivx_video_decoder_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_video_decoder_params_t));

        prms->bitstream_format = TIVX_BITSTREAM_FORMAT_H264;
    }
}
