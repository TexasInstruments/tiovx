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
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_sobel_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelSobelValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelSobelInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelSobelValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[3U];
    vx_uint32 w[3U], h[3U], i;
    vx_df_image fmt[3U], out_fmt;
    vx_border_t border;

    /* Check for NULL */
    if ((NULL == parameters[TIVX_KERNEL_SOBEL_IN_IMG_IDX]) ||
        ((NULL == parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
         (NULL == parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX])))
    {
        status = VX_ERROR_NOT_SUFFICIENT;
    }

    if (VX_SUCCESS == status)
    {
        img[0U] = (vx_image)parameters[TIVX_KERNEL_SOBEL_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
        img[2U] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt[0U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) && (NULL != img[1U]) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for frame sizes */
        if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (VX_DF_IMAGE_S16 != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) && (NULL != img[2U]) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[2U])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[2U], VX_IMAGE_FORMAT, &fmt[2U],
            sizeof(fmt[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_WIDTH, &w[2U], sizeof(w[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_HEIGHT, &h[2U], sizeof(h[2U]));

        /* Check for frame sizes */
        if ((w[0U] != w[2U]) || (h[0U] != h[2U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for format */
        if (VX_DF_IMAGE_S16 != fmt[2U])
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
                VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for sobel\n");
            }
        }
    }

    out_fmt = VX_DF_IMAGE_S16;
    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_SOBEL_MAX_PARAMS; i ++)
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

static vx_status VX_CALLBACK tivxAddKernelSobelInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 num_output_images = 0;
    tivxKernelValidRectParams prms;

    if (num_params != TIVX_KERNEL_SOBEL_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_SOBEL_IN_IMG_IDX];
        if ( (NULL == (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
             (NULL != (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX]) )
        {
            num_output_images = 1;
            prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];
        }
        else if ( (NULL != (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
                  (NULL == (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX]) )
        {
            num_output_images = 1;
            prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
        }
        else if ( (NULL != (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
                  (NULL != (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX]) )
        {
            num_output_images = 2;
            prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
            prms.out_img[1] = (vx_image)parameters[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];
        }

        prms.num_input_images = 1;
        prms.num_output_images = num_output_images;

        prms.top_pad = 1;
        prms.bot_pad = 1;
        prms.left_pad = 1;
        prms.right_pad = 1;
        prms.border_mode = VX_BORDER_UNDEFINED;
        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelSobel3x3(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.sobel_3x3",
                            VX_KERNEL_SOBEL_3x3,
                            NULL,
                            3,
                            tivxAddKernelSobelValidate,
                            tivxAddKernelSobelInitialize,
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
                VX_PARAMETER_STATE_OPTIONAL
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_OPTIONAL
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

    vx_sobel_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSobel3x3(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_sobel_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_sobel_kernel = NULL;

    return status;
}





