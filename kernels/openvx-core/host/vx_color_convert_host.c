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
#include <tivx_kernel_color_convert.h>
#include <TI/tivx_target_kernel.h>


static vx_kernel vx_color_convert_kernel = NULL;

static vx_status tivxCheckFormatAndPlanes(vx_size plane, vx_df_image format);

static vx_status VX_CALLBACK tivxAddKernelColorConvertValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelColorConvertInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status tivxCheckFormatAndPlanes(vx_size plane, vx_df_image format)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    switch(plane)
    {
        case 1:
        {
            switch (format)
            {
                case VX_DF_IMAGE_RGB:
                case VX_DF_IMAGE_RGBX:
                case VX_DF_IMAGE_UYVY:
                case VX_DF_IMAGE_YUYV:
                    status = VX_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        }
        case 2:
        {
            switch (format)
            {
                case VX_DF_IMAGE_NV12:
                case VX_DF_IMAGE_NV21:
                    status = VX_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        }
        case 3:
        {
            switch (format)
            {
                case VX_DF_IMAGE_IYUV:
                case VX_DF_IMAGE_YUV4:
                    status = VX_SUCCESS;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelColorConvertValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_df_image out_fmt;
    vx_uint32 i, w[2U], h[2U];
    vx_df_image src_format, dst_format;
    vx_size src_planes, dst_planes;
    vx_enum src_space;
    vx_rectangle_t rect;

    for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i++)
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
        img[0U] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

        status |= vxQueryImage(img[0U], VX_IMAGE_FORMAT, &src_format, sizeof(src_format));
        status |= vxQueryImage(img[0U], VX_IMAGE_SPACE, &src_space, sizeof(src_space));
        status |= vxQueryImage(img[0U], VX_IMAGE_PLANES, &src_planes, sizeof(src_planes));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));

        status |= vxGetValidRegionImage(img[0U], &rect);

        if (VX_SUCCESS == status)
        {
            status = tivxCheckFormatAndPlanes(src_planes, src_format);
        }
        dst_format = src_format;
    }

    if (VX_SUCCESS == status)
    {
        status |= vxQueryImage(img[1U], VX_IMAGE_FORMAT, &dst_format, sizeof(dst_format));
        status |= vxQueryImage(img[1U], VX_IMAGE_PLANES, &dst_planes, sizeof(dst_planes));

        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        if (VX_SUCCESS == status)
        {
            if(src_format == dst_format)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        if (VX_SUCCESS == status)
        {
            /* Verifies luma channel size */
            if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }

            if(src_format == dst_format)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }

        if (VX_SUCCESS == status)
        {
            status = tivxCheckFormatAndPlanes(dst_planes, dst_format);
        }
    }

    if (VX_SUCCESS == status)
    {
        out_fmt = dst_format;
        for (i = 0U; i < TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vx_enum type = 0;
                vxQueryReference(parameters[i], VX_REFERENCE_TYPE, &type, sizeof(type));
                if (VX_TYPE_IMAGE == type)
                {
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
                        sizeof(out_fmt));
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[0U],
                        sizeof(w[0U]));
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[0U],
                        sizeof(h[0U]));
                }
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK tivxAddKernelColorConvertInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ((num_params != TIVX_KERNEL_COLOR_CONVERT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX])
        || (NULL == parameters[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX]))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_IN_IMG_IDX];
        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_COLOR_CONVERT_OUT_IMG_IDX];

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


vx_status tivxAddKernelColorConvert(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.color_convert",
                            VX_KERNEL_COLOR_CONVERT,
                            NULL,
                            2,
                            tivxAddKernelColorConvertValidate,
                            tivxAddKernelColorConvertInitialize,
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

    vx_color_convert_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelColorConvert(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_color_convert_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_color_convert_kernel = NULL;

    return status;
}


