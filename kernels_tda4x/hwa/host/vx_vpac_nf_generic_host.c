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
#include <TI/tda4x.h>
#include <tivx_hwa_kernels.h>
#include <tivx_kernel_vpac_nf_generic.h>
#include <TI/tivx_target_kernel.h>

static vx_kernel vx_vpac_nf_generic_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[2U];
    vx_uint32 w[2U], h[2U], i;
    vx_df_image fmt[2U];
    vx_convolution conv;
    vx_size size[2U];
    vx_border_t border;

    status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        img[0U] = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_GENERIC_IN_IMG_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_VPAC_NF_GENERIC_OUT_IMG_IDX];
        conv = (vx_convolution)parameters[TIVX_KERNEL_VPAC_NF_GENERIC_IN_CONV_IDX];

        /* Get the image width/heigh and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));

        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if ((VX_DF_IMAGE_U8 != fmt[0U]) &&
            (VX_DF_IMAGE_S16 != fmt[0U]) &&
            (TIVX_DF_IMAGE_P12 != fmt[0U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = vxQueryConvolution(conv, VX_CONVOLUTION_COLUMNS,
            &size[0U], sizeof(size[0u]));
        status |= vxQueryConvolution(conv, VX_CONVOLUTION_ROWS,
            &size[1U], sizeof(size[1U]));

        if ((size[0U] > TIVX_KERNEL_VPAC_NF_GENERIC_CONV_DIM_H) ||
            (size[1U] > TIVX_KERNEL_VPAC_NF_GENERIC_CONV_DIM_V))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        if ((w[0U] < size[0u]) ||
            (h[0U] < size[1u]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    /* Assume output format is same as input format unless otherwise specified */
    fmt[1U] = fmt[0U];

    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));

        /* Check for format */
        if ((VX_DF_IMAGE_U8 != fmt[1U]) &&
            (VX_DF_IMAGE_S16 != fmt[1U]) &&
            (TIVX_DF_IMAGE_P12 != fmt[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual((vx_reference)img[1U])))
    {
        /* Check for frame sizes */
        if ((w[0U] != w[1U]) || (h[0U] != h[1U]))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
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
                VX_PRINT(VX_ZONE_ERROR, "Only replicate border mode is supported for vpac_nf_generic\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        for (i = 0U; i < TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS; i ++)
        {
            if (NULL != metas[i])
            {
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_FORMAT, &fmt[1U],
                    sizeof(fmt[1U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_WIDTH, &w[0U],
                    sizeof(w[0U]));
                vxSetMetaFormatAttribute(metas[i], VX_IMAGE_HEIGHT, &h[0U],
                    sizeof(h[0U]));
            }
        }
    }
    return status;
}

vx_status tivxAddKernel_VPAC_NF_generic(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                            context,
                            "com.ti.hwa.vpac_nf_generic",
                            TIVX_KERNEL_VPAC_NF_GENERIC,
                            NULL,
                            TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS,
                            tivxAddKernelValidate,
                            NULL,
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
                VX_TYPE_CONVOLUTION,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_ARRAY,
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_NF);
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

    vx_vpac_nf_generic_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernel_VPAC_NF_generic(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_nf_generic_kernel;

    /* Kernel is released as part of Remove Kernel */
    status = vxRemoveKernel(kernel);

    vx_vpac_nf_generic_kernel = NULL;

    return status;
}





