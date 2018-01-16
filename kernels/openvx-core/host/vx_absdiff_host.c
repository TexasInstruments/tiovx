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
#include <tivx_kernel_absdiff.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_absdiff_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelAbsDiffValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelAbsDiffInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelAbsDiffValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[TIVX_KERNEL_ABSDIFF_MAX_PARAMS];
    vx_uint32 w[TIVX_KERNEL_ABSDIFF_MAX_PARAMS];
    vx_uint32 h[TIVX_KERNEL_ABSDIFF_MAX_PARAMS];
    vx_uint32 out_w, out_h, i;
    vx_df_image fmt[TIVX_KERNEL_ABSDIFF_MAX_PARAMS], out_fmt;

    if (num != TIVX_KERNEL_ABSDIFF_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; i < TIVX_KERNEL_ABSDIFF_MAX_PARAMS; i ++)
    {
        img[i] = (vx_image)parameters[i];

        /* Check for NULL */
        if (NULL == img[i])
        {
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX],
            sizeof(vx_uint32));
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            sizeof(vx_uint32));
    }
    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual(
            parameters[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            VX_IMAGE_WIDTH, &w[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            sizeof(vx_uint32));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (((VX_DF_IMAGE_S16 != fmt[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX]) &&
             (VX_DF_IMAGE_U8 != fmt[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX])) ||
            ((VX_DF_IMAGE_S16 != fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]) &&
             (VX_DF_IMAGE_U8 != fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX])))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Check for frame sizes */
        if ((w[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX] !=
                w[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]) ||
            (h[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX] !=
                h[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (vx_false_e == tivxIsReferenceVirtual(
            parameters[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX]))
        {
            if ((w[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX] !=
                 w[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX]) ||
                (h[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX] !=
                 h[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if ((VX_DF_IMAGE_S16 == fmt[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX]) ||
            (VX_DF_IMAGE_S16 == fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]))
        {
            out_fmt = VX_DF_IMAGE_S16;
        }
        else
        {
            out_fmt = VX_DF_IMAGE_U8;
        }

        /* If the output format is explicitely set to U8, Set it in
           metadata also */
        status = vxQueryImage(img[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX],
            VX_IMAGE_FORMAT,
            &fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX],
            sizeof(fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]));
        if ((VX_SUCCESS == status) &&
            (VX_DF_IMAGE_U8 == fmt[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]))
        {
            out_fmt = VX_DF_IMAGE_U8;
        }

        out_w = w[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];
        out_h = h[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];

        for (i = 0U; i < TIVX_KERNEL_ABSDIFF_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vx_enum type = 0;
                vxQueryReference(parameters[i], VX_REFERENCE_TYPE, &type, sizeof(type));
                if (VX_TYPE_IMAGE == type)
                {
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &out_fmt,
                        sizeof(out_fmt));
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &out_w,
                        sizeof(out_w));
                    vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &out_h,
                        sizeof(out_h));
                }
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelAbsDiffInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    tivxKernelValidRectParams prms;

    if (num_params != TIVX_KERNEL_ABSDIFF_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; (i < TIVX_KERNEL_ABSDIFF_MAX_PARAMS) &&
            (VX_SUCCESS == status); i ++)
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
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = (vx_image)parameters[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];
        prms.in_img[1] = (vx_image)parameters[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX];
        prms.out_img[0] = (vx_image)parameters[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX];

        prms.num_input_images = 2;
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

vx_status tivxAddKernelAbsDiff(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "org.khronos.openvx.absdiff",
                            VX_KERNEL_ABSDIFF,
                            NULL,
                            3,
                            tivxAddKernelAbsDiffValidate,
                            tivxAddKernelAbsDiffInitialize,
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

    vx_absdiff_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelAbsDiff(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_absdiff_kernel;

    status = vxRemoveKernel(kernel);
    vx_absdiff_kernel = NULL;

    return status;
}





