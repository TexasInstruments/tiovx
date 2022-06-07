/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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
#ifdef BUILD_DMPAC_SDE

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_sde.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_dmpac_sde_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDmpacSdeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxKernelDmpacSdeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxKernelDmpacSdeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_dmpac_sde_params_t params;

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

    uint32_t i;

    if ( (num != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
        left = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        right = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        confidence_histogram = (vx_distribution)parameters[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
        tivxCheckStatus(&status, vxCopyUserDataObject(configuration, 0, sizeof(tivx_dmpac_sde_params_t), &params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryImage(left, (vx_enum)VX_IMAGE_FORMAT, &left_fmt, sizeof(left_fmt)));
        tivxCheckStatus(&status, vxQueryImage(left, (vx_enum)VX_IMAGE_WIDTH, &left_w, sizeof(left_w)));
        tivxCheckStatus(&status, vxQueryImage(left, (vx_enum)VX_IMAGE_HEIGHT, &left_h, sizeof(left_h)));

        tivxCheckStatus(&status, vxQueryImage(right, (vx_enum)VX_IMAGE_FORMAT, &right_fmt, sizeof(right_fmt)));
        tivxCheckStatus(&status, vxQueryImage(right, (vx_enum)VX_IMAGE_WIDTH, &right_w, sizeof(right_w)));
        tivxCheckStatus(&status, vxQueryImage(right, (vx_enum)VX_IMAGE_HEIGHT, &right_h, sizeof(right_h)));

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));

        if (NULL != confidence_histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_BINS, &confidence_histogram_numBins, sizeof(confidence_histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_RANGE, &confidence_histogram_range, sizeof(confidence_histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(confidence_histogram, (vx_enum)VX_DISTRIBUTION_OFFSET, &confidence_histogram_offset, sizeof(confidence_histogram_offset)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_dmpac_sde_params_t)) ||
            (strncmp(configuration_name, "tivx_dmpac_sde_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_dmpac_sde_params_t \n");
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != left_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV12 != left_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != left_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != left_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'left' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if( ((vx_df_image)VX_DF_IMAGE_U8 != right_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV12 != right_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != right_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != right_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'right' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_S16 != output_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if( (left_w != right_w) ||
            (left_w != output_w))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' and 'output' should have the same value for VX_IMAGE_WIDTH\n");
        }
        if( (left_h != right_h) ||
            (left_h != output_h))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' and 'output' should have the same value for VX_IMAGE_HEIGHT\n");
        }

        if (left_fmt != right_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'left' and 'right' should have the same value for VX_IMAGE_FORMAT\n");
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ( (0U != params.median_filter_enable) &&
             (1U != params.median_filter_enable))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter median_filter_enable should be either 0 (Disabled) or 1 (Enable post-processing 5x5 median filter)\n");
        }
        if ( (0U != params.reduced_range_search_enable) &&
             (1U != params.reduced_range_search_enable))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter reduced_range_search_enable should be either 0 (Disabled) or 1 (Enable reduced range search on pixels near right)\n");
        }
        if ( (0U != params.disparity_min) &&
             (1U != params.disparity_min))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter disparity_min should be either 0 (minimum disparity == 0) or 1 (minimum disparity == -3)\n");
        }
        if ( (0U != params.disparity_max) &&
             (1U != params.disparity_max) &&
             (2U != params.disparity_max))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter disparity_max should be either 0 (disparity_min + 63), 1 (disparity_min + 127), or 2 (disparity_min + 191)\n");
        }
        else if ( (2U == params.disparity_max) &&
             (192U > left_w))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter disparity_max should not be 2 (disparity_min + 191) if width < 192\n");
        }
        else if ( (1U == params.disparity_max) &&
             (128U > left_w))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter disparity_max should not be 1 (disparity_min + 127) if width < 128\n");
        }
        else if ( (0U == params.disparity_max) &&
             (64U > left_w))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter disparity_max should not be 0 (disparity_min + 63) if width < 64\n");
        }
        else
        {
            /* do nothing */
        }
        if (255U < params.threshold_left_right)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter threshold_left_right should be between 0 and 255 inclusive\n");
        }
        if ( (0U != params.texture_filter_enable) &&
             (1U != params.texture_filter_enable))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter texture_filter_enable should be either 0 (Disabled) or 1 (Enable texture based filtering)\n");
        }
        else if ( (1U == params.texture_filter_enable) &&
             (255U < params.threshold_texture))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter threshold_texture should be between 0 and 255 inclusive if texture_filter_enable is 1\n");
        }
        else
        {
            /* do nothing */
        }
        if (127U < params.aggregation_penalty_p1)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter aggregation_penalty_p1 should be between 0 and 127 inclusive\n");
        }
        if (255U < params.aggregation_penalty_p2)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter aggregation_penalty_p2 should be between 0 and 255 inclusive\n");
        }
        for(i = 0U; i < 8U; i++)
        {
            if (127U < params.confidence_score_map[i])
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter confidence_score_map should contain values between 0 and 127 inclusive\n");
            }
            else if ((7U > i) && (126U < params.confidence_score_map[i]))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter confidence_score_map (0-6) should contain values between 0 and 126 inclusive\n");
            }
            else if ((0U != i) && (params.confidence_score_map[i] <= params.confidence_score_map[i - 1U]))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter confidence_score_map should contain strictly increasing values\n");
            }
            else
            {
                /* do nothing */
            }

            if(status != (vx_status)VX_SUCCESS)
            {
                break;
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelDmpacSdeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
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
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;
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
                    TIVX_KERNEL_DMPAC_SDE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS,
                    tivxKernelDmpacSdeValidate,
                    tivxKernelDmpacSdeInitialize,
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
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_DISTRIBUTION,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DMPAC_SDE);
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

void tivx_dmpac_sde_params_init(tivx_dmpac_sde_params_t *prms)
{
    uint16_t i;

    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_dmpac_sde_params_t));

        prms->median_filter_enable = 0u;
        prms->reduced_range_search_enable = 0u;
        prms->disparity_min = 0u;
        prms->disparity_max = 0u;
        prms->threshold_left_right = 0u;
        prms->texture_filter_enable = 0u;
        prms->threshold_texture = 0u;
        prms->aggregation_penalty_p1 = 0u;
        prms->aggregation_penalty_p2 = 0u;

        for(i = 0u; i < 8u; i++)
        {
            prms->confidence_score_map[i] = i;
        }
    }
}

#endif /* BUILD_DMPAC_SDE */