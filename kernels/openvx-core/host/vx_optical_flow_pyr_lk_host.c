/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
#include <tivx_kernel_optical_flow_pyr_lk.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_optical_flow_pyr_lk_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_pyramid old_images = NULL;
    vx_uint32 old_images_w;
    vx_uint32 old_images_h;
    vx_size old_images_levels;
    vx_float32 old_images_scale;
    vx_df_image old_images_fmt;

    vx_pyramid new_images = NULL;
    vx_uint32 new_images_w;
    vx_uint32 new_images_h;
    vx_size new_images_levels;
    vx_float32 new_images_scale;
    vx_df_image new_images_fmt;

    vx_array old_points = NULL;
    vx_enum old_points_item_type;
    vx_size old_points_capacity;

    vx_array new_points_estimates = NULL;
    vx_enum new_points_estimates_item_type;
    vx_size new_points_estimates_capacity;

    vx_array new_points = NULL;
    vx_enum new_points_item_type;
    vx_size new_points_capacity;

    vx_scalar termination = NULL;
    vx_enum termination_scalar_type;

    vx_scalar epsilon = NULL;
    vx_enum epsilon_scalar_type;

    vx_scalar num_iterations = NULL;
    vx_enum num_iterations_scalar_type;

    vx_scalar use_initial_estimate = NULL;
    vx_enum use_initial_estimate_scalar_type;

    vx_scalar window_dimension = NULL;
    vx_size window_dimension_val;

    vx_border_t border;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_ESTIMATES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_TERMINATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_EPSILON_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NUM_ITERATIONS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_USE_INITIAL_ESTIMATE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        old_images = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX];
        new_images = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX];
        old_points = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX];
        new_points_estimates = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_ESTIMATES_IDX];
        new_points = (vx_array)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX];
        termination = (vx_scalar)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_TERMINATION_IDX];
        epsilon = (vx_scalar)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_EPSILON_IDX];
        num_iterations = (vx_scalar)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NUM_ITERATIONS_IDX];
        use_initial_estimate = (vx_scalar)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_USE_INITIAL_ESTIMATE_IDX];
        window_dimension = (vx_scalar)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_WIDTH, &old_images_w, sizeof(old_images_w)));
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_HEIGHT, &old_images_h, sizeof(old_images_h)));
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_FORMAT, &old_images_fmt, sizeof(old_images_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_LEVELS, &old_images_levels, sizeof(old_images_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_SCALE, &old_images_scale, sizeof(old_images_scale)));

        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_WIDTH, &new_images_w, sizeof(new_images_w)));
        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_HEIGHT, &new_images_h, sizeof(new_images_h)));
        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_FORMAT, &new_images_fmt, sizeof(new_images_fmt)));
        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_LEVELS, &new_images_levels, sizeof(new_images_levels)));
        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_SCALE, &new_images_scale, sizeof(new_images_scale)));

        tivxCheckStatus(&status, vxQueryArray(old_points, (vx_enum)VX_ARRAY_ITEMTYPE, &old_points_item_type, sizeof(old_points_item_type)));
        tivxCheckStatus(&status, vxQueryArray(old_points, (vx_enum)VX_ARRAY_CAPACITY, &old_points_capacity, sizeof(old_points_capacity)));

        tivxCheckStatus(&status, vxQueryArray(new_points_estimates, (vx_enum)VX_ARRAY_ITEMTYPE, &new_points_estimates_item_type, sizeof(new_points_estimates_item_type)));
        tivxCheckStatus(&status, vxQueryArray(new_points_estimates, (vx_enum)VX_ARRAY_CAPACITY, &new_points_estimates_capacity, sizeof(new_points_estimates_capacity)));

        tivxCheckStatus(&status, vxQueryArray(new_points, (vx_enum)VX_ARRAY_ITEMTYPE, &new_points_item_type, sizeof(new_points_item_type)));
        tivxCheckStatus(&status, vxQueryArray(new_points, (vx_enum)VX_ARRAY_CAPACITY, &new_points_capacity, sizeof(new_points_capacity)));

        tivxCheckStatus(&status, vxCopyScalar(termination, &termination_scalar_type, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryScalar(epsilon, (vx_enum)VX_SCALAR_TYPE, &epsilon_scalar_type, sizeof(epsilon_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(num_iterations, (vx_enum)VX_SCALAR_TYPE, &num_iterations_scalar_type, sizeof(num_iterations_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(use_initial_estimate, (vx_enum)VX_SCALAR_TYPE, &use_initial_estimate_scalar_type, sizeof(use_initial_estimate_scalar_type)));

        tivxCheckStatus(&status, vxCopyScalar(window_dimension, &window_dimension_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)new_points);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8 != old_images_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' should be a pyramid of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_df_image)VX_DF_IMAGE_U8 != new_images_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'new_images' should be a pyramid of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_enum)VX_TYPE_KEYPOINT != old_points_item_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_points' should be an array of type:\n VX_TYPE_KEYPOINT \n");
        }

        if ((vx_enum)VX_TYPE_KEYPOINT != new_points_estimates_item_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'new_points_estimates' should be an array of type:\n VX_TYPE_KEYPOINT \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_enum)VX_TYPE_KEYPOINT != new_points_item_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'new_points' should be an array of type:\n VX_TYPE_KEYPOINT \n");
            }
        }

        if (((vx_enum)VX_TERM_CRITERIA_ITERATIONS != termination_scalar_type) &&
            ((vx_enum)VX_TERM_CRITERIA_EPSILON != termination_scalar_type) &&
            ((vx_enum)VX_TERM_CRITERIA_BOTH != termination_scalar_type))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'termination' should be a scalar of type:\n VX_TERM_CRITERIA_ITERATIONS or VX_TERM_CRITERIA_EPSILON or VX_TERM_CRITERIA_BOTH \n");
        }

        if ((vx_enum)VX_TYPE_FLOAT32 != epsilon_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'epsilon' should be a scalar of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((vx_enum)VX_TYPE_UINT32 != num_iterations_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'num_iterations' should be a scalar of type:\n VX_TYPE_UINT32 \n");
        }

        if ((vx_enum)VX_TYPE_BOOL != use_initial_estimate_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'use_initial_estimate' should be a scalar of type:\n VX_TYPE_BOOL \n");
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (old_images_levels != new_images_levels)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' and 'new_images' should have the same value for VX_PYRAMID_LEVELS\n");
        }

        if (old_images_scale != new_images_scale)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' and 'new_images' should have the same value for VX_PYRAMID_SCALE \n");
        }

        if (old_images_w != new_images_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' and 'new_images' should have the same value for VX_PYRAMID_WIDTH \n");
        }

        if (old_images_h != new_images_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' and 'new_images' should have the same value for VX_PYRAMID_HEIGHT \n");
        }

        if (old_points_capacity != new_points_estimates_capacity)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_points' and 'new_points_estimates' should have the same value for VX_ARRAY_CAPACITY \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (old_points_capacity != new_points_capacity)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'old_points' and 'new_points' should have the same value for VX_ARRAY_CAPACITY \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
#if 0
        if (TIVX_CONTEXT_MAX_OPTICALFLOWPYRLK_DIM < window_dimension_val)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'window_dimension' must not be greater than VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION \n");
        }
#endif
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED != border.mode)
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for optical flow \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX], (vx_enum)VX_ARRAY_ITEMTYPE, &old_points_item_type, sizeof(old_points_item_type));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX], (vx_enum)VX_ARRAY_CAPACITY, &old_points_capacity, sizeof(old_points_capacity));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelOpticalFlowPyrLkInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;
    vx_pyramid old_images = NULL;
    vx_size old_images_levels;
    vx_pyramid new_images = NULL;
    vx_size new_images_levels;

    vx_image old_image;
    vx_image new_image;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    vx_uint32 i;

    if ( (num_params != TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_ESTIMATES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_TERMINATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_EPSILON_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NUM_ITERATIONS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_USE_INITIAL_ESTIMATE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_WINDOW_DIMENSION_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        old_images = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_OLD_IMAGES_IDX];
        new_images = (vx_pyramid)parameters[TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_NEW_IMAGES_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(old_images, (vx_enum)VX_PYRAMID_LEVELS, &old_images_levels, sizeof(old_images_levels)));

        tivxCheckStatus(&status, vxQueryPyramid(new_images, (vx_enum)VX_PYRAMID_LEVELS, &new_images_levels, sizeof(new_images_levels)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)new_images);

#endif

    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (old_images_levels != new_images_levels)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'old_images' and 'new_images' should have the same value for VX_PYRAMID_LEVELS\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        if((vx_bool)vx_false_e == is_virtual)
        {
            for (i = 0U; i < old_images_levels; i++)
            {
                old_image = vxGetPyramidLevel(old_images, i);
                new_image = vxGetPyramidLevel(new_images, i);

                prms.in_img[0U] = old_image;
                prms.in_img[1U] = new_image;

                prms.num_input_images = 2U;
                prms.num_output_images = 0U;

                prms.top_pad = 0U;
                prms.bot_pad = 0U;
                prms.left_pad = 0U;
                prms.right_pad = 0U;
                prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

                tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));

                tivxCheckStatus(&status, vxReleaseImage(&old_image));
                tivxCheckStatus(&status, vxReleaseImage(&new_image));
            }
        }
    }

    return status;
}

vx_status tivxAddKernelOpticalFlowPyrLk(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.optical_flow_pyr_lk",
                (vx_enum)VX_KERNEL_OPTICAL_FLOW_PYR_LK,
                NULL,
                TIVX_KERNEL_OPTICAL_FLOW_PYR_LK_MAX_PARAMS,
                tivxAddKernelOpticalFlowPyrLkValidate,
                tivxAddKernelOpticalFlowPyrLkInitialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);

    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_ARRAY,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SIZE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
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
    vx_optical_flow_pyr_lk_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelOpticalFlowPyrLk(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_optical_flow_pyr_lk_kernel;

    status = vxRemoveKernel(kernel);
    vx_optical_flow_pyr_lk_kernel = NULL;

    return status;
}


