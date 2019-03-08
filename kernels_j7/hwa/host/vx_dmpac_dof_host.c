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

#include "TI/tivx.h"
#include "TI/j7.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_dmpac_dof_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDmpacDofValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelDmpacDofInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelDmpacDof(vx_context context);
vx_status tivxRemoveKernelDmpacDof(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelDmpacDofValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_pyramid input_current = NULL;
    vx_df_image input_current_fmt;
    vx_size input_current_levels;
    vx_float32 input_current_scale;
    vx_uint32 input_current_w, input_current_h;

    vx_pyramid input_reference = NULL;
    vx_df_image input_reference_fmt;
    vx_size input_reference_levels;
    vx_float32 input_reference_scale;
    vx_uint32 input_reference_w, input_reference_h;

    vx_image flow_vector_in = NULL;
    vx_df_image flow_vector_in_fmt;
    vx_uint32 flow_vector_in_w, flow_vector_in_h;

    vx_image sparse_of_map = NULL;
    vx_df_image sparse_of_map_fmt;
    vx_uint32 sparse_of_map_w, sparse_of_map_h;

    vx_image flow_vector_out = NULL;
    vx_df_image flow_vector_out_fmt;
    vx_uint32 flow_vector_out_w, flow_vector_out_h;

    vx_distribution confidence_histogram = NULL;
    vx_int32 confidence_histogram_offset = 0;
    vx_uint32 confidence_histogram_range = 0;
    vx_size confidence_histogram_numBins = 0;

    if ( (num != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];
        input_current = (vx_pyramid)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_reference = (vx_pyramid)parameters[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_map = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out = (vx_image)parameters[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram = (vx_distribution)parameters[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryPyramid(input_current, VX_PYRAMID_FORMAT, &input_current_fmt, sizeof(input_current_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, VX_PYRAMID_LEVELS, &input_current_levels, sizeof(input_current_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, VX_PYRAMID_SCALE, &input_current_scale, sizeof(input_current_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, VX_PYRAMID_WIDTH, &input_current_w, sizeof(input_current_w)));
        tivxCheckStatus(&status, vxQueryPyramid(input_current, VX_PYRAMID_HEIGHT, &input_current_h, sizeof(input_current_h)));

        tivxCheckStatus(&status, vxQueryPyramid(input_reference, VX_PYRAMID_FORMAT, &input_reference_fmt, sizeof(input_reference_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, VX_PYRAMID_LEVELS, &input_reference_levels, sizeof(input_reference_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, VX_PYRAMID_SCALE, &input_reference_scale, sizeof(input_reference_scale)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, VX_PYRAMID_WIDTH, &input_reference_w, sizeof(input_reference_w)));
        tivxCheckStatus(&status, vxQueryPyramid(input_reference, VX_PYRAMID_HEIGHT, &input_reference_h, sizeof(input_reference_h)));

        if (NULL != flow_vector_in)
        {
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, VX_IMAGE_FORMAT, &flow_vector_in_fmt, sizeof(flow_vector_in_fmt)));
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, VX_IMAGE_WIDTH, &flow_vector_in_w, sizeof(flow_vector_in_w)));
            tivxCheckStatus(&status, vxQueryImage(flow_vector_in, VX_IMAGE_HEIGHT, &flow_vector_in_h, sizeof(flow_vector_in_h)));
        }

        if (NULL != sparse_of_map)
        {
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, VX_IMAGE_FORMAT, &sparse_of_map_fmt, sizeof(sparse_of_map_fmt)));
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, VX_IMAGE_WIDTH, &sparse_of_map_w, sizeof(sparse_of_map_w)));
            tivxCheckStatus(&status, vxQueryImage(sparse_of_map, VX_IMAGE_HEIGHT, &sparse_of_map_h, sizeof(sparse_of_map_h)));
        }

        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, VX_IMAGE_FORMAT, &flow_vector_out_fmt, sizeof(flow_vector_out_fmt)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, VX_IMAGE_WIDTH, &flow_vector_out_w, sizeof(flow_vector_out_w)));
        tivxCheckStatus(&status, vxQueryImage(flow_vector_out, VX_IMAGE_HEIGHT, &flow_vector_out_h, sizeof(flow_vector_out_h)));

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
        if ((configuration_size != sizeof(tivx_dmpac_dof_params_t)) ||
            (strncmp(configuration_name, "tivx_dmpac_dof_params_t", sizeof(configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_dmpac_dof_params_t \n");
        }

        if( (VX_DF_IMAGE_U8 != input_current_fmt) &&
            (VX_DF_IMAGE_U16 != input_current_fmt) &&
            (TIVX_DF_IMAGE_P12 != input_current_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_current' should be a pyramid of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if( (VX_DF_IMAGE_U8 != input_reference_fmt) &&
            (VX_DF_IMAGE_U16 != input_reference_fmt) &&
            (TIVX_DF_IMAGE_P12 != input_reference_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_reference' should be a pyramid of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if (NULL != flow_vector_in)
        {
            if (VX_DF_IMAGE_U32 != flow_vector_in_fmt)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'flow_vector_in' should be an image of type:\n VX_DF_IMAGE_U32 \n");
            }
        }

        if (NULL != sparse_of_map)
        {
            if (VX_DF_IMAGE_U8 != sparse_of_map_fmt)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'sparse_of_map' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }

        if (VX_DF_IMAGE_U32 != flow_vector_out_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'flow_vector_out' should be an image of type:\n VX_DF_IMAGE_U32 \n");
        }

    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if (VX_SUCCESS == status)
    {
        if (input_current_levels != input_reference_levels)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_LEVELS\n");
        }
        if (input_current_scale != input_reference_scale)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_SCALE\n");
        }
        if (input_current_w != input_reference_w)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_WIDTH\n");
        }
        if (input_current_h != input_reference_h)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_HEIGHT\n");
        }
        if (input_current_fmt != input_reference_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'input_reference' should have the same value for VX_PYRAMID_FORMAT\n");
        }

        if (input_current_w != flow_vector_out_w)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'flow_vector_out' should have the same value for VX_IMAGE_WIDTH\n");
        }
        if (input_current_h != flow_vector_out_h)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'input_current' and 'flow_vector_out' should have the same value for VX_IMAGE_HEIGHT\n");
        }

        if (NULL != flow_vector_in)
        {
            if (flow_vector_in_w != input_current_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector_in' and 'input_current' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (flow_vector_in_h != input_current_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'flow_vector_in' and 'input_current' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != sparse_of_map)
        {
            if (sparse_of_map_w != input_current_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'sparse_of_map' and 'input_current' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (sparse_of_map_h != input_current_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'sparse_of_map' and 'input_current' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if(input_current_levels > TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Number of pyramid levels %d exceeds max supported values of %d !!!\n",
                input_current_levels,
                TIVX_KERNEL_DMPAC_DOF_MAX_LEVELS
                );
        }

        if(input_current_scale != VX_SCALE_PYRAMID_HALF )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Scale of pyramid %3.1f does not match required scale of %3.1f!!!\n",
                input_current_scale,
                VX_SCALE_PYRAMID_HALF
                );
        }

        if((input_current_w > 2048U) || (input_current_h > 1024U))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Pyramid base image WxH exceeds max supported limit !!!\n"
                );
        }

        if( ((input_current_w % (1U<<(uint32_t)input_current_levels)) != 0) &&
            ((input_current_h % (1U<<(uint32_t)input_current_levels)) != 0)
          )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Pyramid base image WxH of %d %d MUST be aligned to %d pixels !!!\n",
                input_current_w,
                input_current_h,
                1U<<(uint32_t)input_current_levels
                );
        }

        if(confidence_histogram != NULL)
        {
            if((confidence_histogram_numBins!=TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS) ||
               (confidence_histogram_range!=TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS) ||
               (confidence_histogram_offset!=0))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "DMPAC_DOF: Confidence measure histogram meta properties "
                    "(bins=%d, offset=%d, range=%d) do NOT match expected meta properties (bins=%d, offset=0, range=%d)!!!\n",
                    confidence_histogram_numBins, confidence_histogram_range, confidence_histogram_offset,
                    TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS, TIVX_KERNEL_DMPAC_DOF_MAX_CONFIDENCE_HIST_BINS
                    );
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDmpacDofInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status tivxAddKernelDmpacDof(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_DMPAC_DOF_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS,
                    tivxAddKernelDmpacDofValidate,
                    tivxAddKernelDmpacDofInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_PYRAMID,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_PYRAMID,
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_DMPAC_DOF);
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
    vx_dmpac_dof_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDmpacDof(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_dmpac_dof_kernel;

    status = vxRemoveKernel(kernel);
    vx_dmpac_dof_kernel = NULL;

    return status;
}


