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
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_channel_extract_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelChannelExtractValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelChannelExtractInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelChannelExtractValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image img[2U] = {NULL};
    vx_scalar scalar;
    vx_df_image fmt[2U];
    vx_df_image out_fmt;
    vx_enum type, channel;
    vx_uint32 i, w[2U], h[2U], out_w, out_h;

    for (i = 0U; i < TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS; i ++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];
        scalar = (vx_scalar)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));

        status |= vxQueryScalar(scalar, VX_SCALAR_TYPE, &type, sizeof(type));

        status |= vxCopyScalar(scalar, &channel, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    }

    if (VX_SUCCESS == status)
    {
        if (type != VX_TYPE_ENUM)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        if (   (fmt[0U] != VX_DF_IMAGE_RGB)
            && (fmt[0U] != VX_DF_IMAGE_RGBX)
            && (fmt[0U] != VX_DF_IMAGE_NV12)
            && (fmt[0U] != VX_DF_IMAGE_NV21)
            && (fmt[0U] != VX_DF_IMAGE_UYVY)
            && (fmt[0U] != VX_DF_IMAGE_YUYV)
            && (fmt[0U] != VX_DF_IMAGE_IYUV)
            && (fmt[0U] != VX_DF_IMAGE_YUV4)
           )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        out_fmt = VX_DF_IMAGE_U8;
        out_w = w[0U];
        out_h = h[0U];
        /* output WxH MUST be equal to input WxH except in below conditions */
        switch(fmt[0U])
        {
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
                if((channel == VX_CHANNEL_U) || (channel == VX_CHANNEL_V))
                {
                    out_w = out_w/2;
                    out_h = out_h/2;
                }
                break;
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_YUYV:
                if((channel == VX_CHANNEL_U) || (channel == VX_CHANNEL_V))
                {
                    out_w = out_w/2;
                }
                break;
            default:
                break;
        }
    }

    if (VX_SUCCESS == status)
    {
        if(vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U]))
        {
            /* Get the image width/heigh and format */
            status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U], sizeof(fmt[1U]));
            status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
            status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

            /* Check for frame sizes */
            if ((out_w != w[1U]) || (out_h != h[1U]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            if ( fmt[1U] != out_fmt )
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        i = TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX;

        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
            sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &out_w,
            sizeof(w[0U]));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &out_h,
            sizeof(h[0U]));
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelChannelExtractInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ((num_params != TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX]))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];

        prms.num_input_images = 1;
        prms.num_output_images = 1;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}


vx_status tivxAddKernelChannelExtract(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.channel_extract",
                VX_KERNEL_CHANNEL_EXTRACT,
                NULL,
                TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS,
                tivxAddKernelChannelExtractValidate,
                tivxAddKernelChannelExtractInitialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);
    if (status == VX_SUCCESS)
    {
        index = 0;

        if (status == VX_SUCCESS)
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
                        VX_INPUT,
                        VX_TYPE_ENUM,
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
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
    vx_channel_extract_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelChannelExtract(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_channel_extract_kernel;

    status = vxRemoveKernel(kernel);
    vx_channel_extract_kernel = NULL;

    return status;
}


