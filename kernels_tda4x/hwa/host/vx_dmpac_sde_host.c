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

#include "TI/tivx.h"
#include "TI/tda4x.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_sde.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_dmpac_sde_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDmpacSdeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelDmpacSdeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelDmpacSdeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[3U] = {NULL};
    vx_array array_0 = {NULL};
    vx_enum item_type_0;
    vx_size capacity_0;
    vx_size item_size_0;
    vx_distribution distribution_1 = {NULL};
    vx_int32 offset_1 = 0;
    vx_uint32 range_1 = 0;
    vx_size numBins_1 = 0;
    vx_df_image fmt[3U] = {0u, 0u, 0u};
    vx_df_image out_fmt = VX_DF_IMAGE_U8;
    vx_uint32 w[3U], h[3U];

    status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS-1);

    if (VX_SUCCESS == status)
    {
        img[0U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        array_0 = (vx_array)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
        img[2U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        distribution_1 = (vx_distribution)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];

    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));
    }

    if (VX_SUCCESS == status)
    {
        status |= vxQueryArray(array_0, VX_ARRAY_ITEMTYPE, &item_type_0, sizeof(item_type_0));
        status |= vxQueryArray(array_0, VX_ARRAY_CAPACITY, &capacity_0, sizeof(capacity_0));
        status |= vxQueryArray(array_0, VX_ARRAY_ITEMSIZE, &item_size_0, sizeof(item_size_0));
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[2U], VX_IMAGE_FORMAT, &fmt[2U],
            sizeof(fmt[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_WIDTH, &w[2U], sizeof(w[2U]));
        status |= vxQueryImage(img[2U], VX_IMAGE_HEIGHT, &h[2U], sizeof(h[2U]));
    }

    if ((VX_SUCCESS == status) && (NULL != distribution_1))
    {
        status |= vxQueryDistribution(distribution_1, VX_DISTRIBUTION_BINS, &numBins_1, sizeof(numBins_1));
        status |= vxQueryDistribution(distribution_1, VX_DISTRIBUTION_RANGE, &range_1, sizeof(range_1));
        status |= vxQueryDistribution(distribution_1, VX_DISTRIBUTION_OFFSET, &offset_1, sizeof(offset_1));
    }

    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidateInputSize(w[0U], w[1U], h[0U], h[1U]);
    }

    /* Check possible input image formats */
    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidatePossibleFormat(fmt[0U], VX_DF_IMAGE_U8) &
                 tivxKernelValidatePossibleFormat(fmt[0U], VX_DF_IMAGE_U16) &
                 tivxKernelValidatePossibleFormat(fmt[0U], TIVX_DF_IMAGE_P12);
    }

    /* Check possible input image formats */
    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidatePossibleFormat(fmt[1U], VX_DF_IMAGE_U8) &
                 tivxKernelValidatePossibleFormat(fmt[1U], VX_DF_IMAGE_U16) &
                 tivxKernelValidatePossibleFormat(fmt[1U], TIVX_DF_IMAGE_P12);
    }

    /* Check possible output image formats */
    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidatePossibleFormat(fmt[2U], VX_DF_IMAGE_S16);
    }

    /* Check size of configuration data structure (array) */
    if (VX_SUCCESS == status)
    {
        if( item_size_0 != sizeof(tivx_dmpac_sde_params_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an array of a user struct of type:\n tivx_dmpac_sde_params_t \n");
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidateOutputSize(w[0U], w[2U], h[0U], h[2U], img[2U]);
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelSetMetas(metas, TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS, out_fmt, w[0U], h[0U]);
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDmpacSdeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS-1);
    }

    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];

        prms.num_input_images = 2;
        prms.num_output_images = 1;

        /* < DEVELOPER_TODO: (Optional) Set padding values based on valid region if border mode is */
        /*                    set to VX_BORDER_UNDEFINED and remove the #if 0 and #endif lines. */
        /*                    Else, remove this entire #if 0 ... #endif block > */
        #if 0
        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;
        #endif

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelDmpacSde(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "com.ti.hwa.dmpac_sde",
                TIVX_KERNEL_DMPAC_SDE,
                NULL,
                TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS,
                tivxAddKernelDmpacSdeValidate,
                tivxAddKernelDmpacSdeInitialize,
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
                        VX_TYPE_ARRAY,
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
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_DISTRIBUTION,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DMPAC_SDE);
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
    vx_dmpac_sde_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDmpacSde(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_dmpac_sde_kernel;

    status = vxRemoveKernel(kernel);
    vx_dmpac_sde_kernel = NULL;

    return status;
}


