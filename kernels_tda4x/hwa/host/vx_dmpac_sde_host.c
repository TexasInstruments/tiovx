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
vx_status tivxAddKernelDmpacSde(vx_context context);
vx_status tivxRemoveKernelDmpacSde(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelDmpacSdeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_array configuration = NULL;
    vx_enum configuration_item_type;
    vx_size configuration_capacity, configuration_item_size;

    vx_image left = NULL;
    vx_df_image left_fmt;
    vx_uint32 left_w, left_h;

    vx_image right = NULL;
    vx_df_image right_fmt;
    vx_uint32 right_w, right_h;

    vx_image output = NULL;
    vx_df_image output_fmt;
    vx_uint32 output_w, output_h;

    vx_distribution confidence_histogram = NULL;
    vx_int32 confidence_histogram_offset = 0;
    vx_uint32 confidence_histogram_range = 0;
    vx_size confidence_histogram_numBins = 0;

    if ( (num != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (const vx_array)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
        left = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        right = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        output = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        confidence_histogram = (const vx_distribution)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMTYPE, &configuration_item_type, sizeof(configuration_item_type)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_CAPACITY, &configuration_capacity, sizeof(configuration_capacity)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMSIZE, &configuration_item_size, sizeof(configuration_item_size)));

        tivxCheckStatus(&status, vxQueryImage(left, VX_IMAGE_FORMAT, &left_fmt, sizeof(left_fmt)));
        tivxCheckStatus(&status, vxQueryImage(left, VX_IMAGE_WIDTH, &left_w, sizeof(left_w)));
        tivxCheckStatus(&status, vxQueryImage(left, VX_IMAGE_HEIGHT, &left_h, sizeof(left_h)));

        tivxCheckStatus(&status, vxQueryImage(right, VX_IMAGE_FORMAT, &right_fmt, sizeof(right_fmt)));
        tivxCheckStatus(&status, vxQueryImage(right, VX_IMAGE_WIDTH, &right_w, sizeof(right_w)));
        tivxCheckStatus(&status, vxQueryImage(right, VX_IMAGE_HEIGHT, &right_h, sizeof(right_h)));

        tivxCheckStatus(&status, vxQueryImage(output, VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output, VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));

        if (NULL != confidence_histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, VX_DISTRIBUTION_BINS, &confidence_histogram_numBins, sizeof(confidence_histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, VX_DISTRIBUTION_RANGE, &confidence_histogram_range, sizeof(confidence_histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, VX_DISTRIBUTION_OFFSET, &confidence_histogram_offset, sizeof(confidence_histogram_offset)));
        }
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ( configuration_item_size != sizeof(tivx_dmpac_sde_params_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an array of type:\n tivx_dmpac_sde_params_t \n");
        }

        if( (VX_DF_IMAGE_U8 != left_fmt) &&
            (VX_DF_IMAGE_U16 != left_fmt) &&
            (TIVX_DF_IMAGE_P12 != left_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'left' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if( (VX_DF_IMAGE_U8 != right_fmt) &&
            (VX_DF_IMAGE_U16 != right_fmt) &&
            (TIVX_DF_IMAGE_P12 != right_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'right' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if (VX_DF_IMAGE_S16 != output_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if (VX_SUCCESS == status)
    {
        if( (left_w != right_w) ||
            (left_w != output_w))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' and 'output' should have the same value for VX_IMAGE_WIDTH\n");
        }
        if( (left_h != right_h) ||
            (left_h != output_h))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' and 'output' should have the same value for VX_IMAGE_HEIGHT\n");
        }

        if (left_fmt != right_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' should have the same value for VX_IMAGE_FORMAT\n");
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDmpacSdeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        prms.in_img[1U] = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        prms.out_img[0U] = (const vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];

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


