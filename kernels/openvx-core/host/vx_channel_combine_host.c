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

static vx_kernel vx_channel_combine_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelChannelCombineValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelChannelCombineInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelChannelCombineValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image img[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_df_image fmt[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_uint32 w[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_uint32 h[TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS];
    vx_df_image out_fmt;
    vx_uint32 out_w, out_h;
    vx_uint32 i;
    vx_uint16 in_channel, out_channel;

    in_channel = 0;
    for (i = 0U; i < TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS; i ++)
    {
        img[i] = NULL;
        w[i] = 0;
        h[i] = 0;
        fmt[i] = VX_DF_IMAGE_VIRT;

        if (NULL == parameters[i])
        {
            /* Check for NULL for required parameters */
            if((i == TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX)
               || (i == TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX)
               || (i == TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX)
                )
            {
                status = VX_ERROR_NO_MEMORY;
                break;
            }
        }
        else
        {
            img[i] = (vx_image)parameters[i];

            /* Get the image width/heigh and format */
            status |= vxQueryImage(img[i], VX_IMAGE_FORMAT, &fmt[i],
                sizeof(fmt[i]));

            status |= vxQueryImage(img[i], VX_IMAGE_WIDTH, &w[i], sizeof(w[i]));
            status |= vxQueryImage(img[i], VX_IMAGE_HEIGHT, &h[i], sizeof(h[i]));

            if(i == TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX)
            {

            }
            else
            {
                in_channel++;

                if(fmt[i] != VX_DF_IMAGE_U8)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }

    out_w = w[0U];
    out_h = h[0U];
    out_fmt = fmt[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX];

    /* check if output format matches number of planes given as input */
    if( status == VX_SUCCESS)
    {
        out_channel = 0;
        switch(out_fmt)
        {
            case VX_DF_IMAGE_RGBX:
                out_channel = 4;
                break;
            case VX_DF_IMAGE_RGB:
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_YUV4:
                out_channel = 3;
                break;
            default:
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }
    }

    if ( status == VX_SUCCESS )
    {
        if(in_channel!=out_channel)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    if ( status == VX_SUCCESS )
    {
        switch(out_fmt)
        {
            case VX_DF_IMAGE_YUV4:
            case VX_DF_IMAGE_RGBX:
            case VX_DF_IMAGE_RGB:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]
                        ||
                        h[i] != h[0]
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]/2
                        ||
                        h[i] != h[0]/2
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_UYVY:
                for(i=1U; i<in_channel; i++)
                {
                    if( w[i] != w[0]/2
                        ||
                        h[i] != h[0]
                    )
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                break;
            default:
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }

    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX])))
    {
        /* Check for frame sizes */
        if ((out_w != w[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX]) || (out_h != h[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        i = TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX;

        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
            sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &out_w,
            sizeof(w[0U]));
        vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &out_h,
            sizeof(h[0U]));
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelChannelCombineInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ((num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX])
        || (NULL == parameters[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX]))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX];
        prms.in_img[1] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX];
        prms.num_input_images = 2;

        if (NULL != parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC2_IDX])
        {
            prms.in_img[2] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC2_IDX];
            prms.num_input_images = 3;
        }

        if (NULL != parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC3_IDX])
        {
            prms.in_img[3] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_SRC3_IDX];
            prms.num_input_images = 4;
        }

        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX];


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


vx_status tivxAddKernelChannelCombine(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.channel_combine",
                VX_KERNEL_CHANNEL_COMBINE,
                NULL,
                TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS,
                tivxAddKernelChannelCombineValidate,
                tivxAddKernelChannelCombineInitialize,
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
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
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


