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
#include <tivx_kernel_harris_corners.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_harris_corners_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelHarrisCornersValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelHarrisCornersInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelHarrisCornersValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_scalar strength_thresh = NULL;
    vx_enum strength_thresh_scalar_type;

    vx_scalar min_distance = NULL;
    vx_enum min_distance_scalar_type;

    vx_scalar sensitivity = NULL;
    vx_enum sensitivity_scalar_type;

    vx_scalar gradient_size = NULL;
    vx_enum gradient_size_scalar_type;
    vx_int32 gradient_size_val;

    vx_scalar block_size = NULL;
    vx_enum block_size_scalar_type;
    vx_int32 block_size_val;

    vx_array corners = NULL;
    vx_enum corners_item_type;
    vx_size corners_capacity;

    vx_scalar num_corners = NULL;
    vx_enum num_corners_scalar_type;

    vx_border_t border;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( ((num != TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS) && (num != (TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS - 1U)))
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_STRENGTH_THRESH_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_MIN_DISTANCE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_SENSITIVITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_GRADIENT_SIZE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_BLOCK_SIZE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input = (vx_image)parameters[TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX];
        strength_thresh = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_STRENGTH_THRESH_IDX];
        min_distance = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_MIN_DISTANCE_IDX];
        sensitivity = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_SENSITIVITY_IDX];
        gradient_size = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_GRADIENT_SIZE_IDX];
        block_size = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_BLOCK_SIZE_IDX];
        corners = (vx_array)parameters[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX];
        num_corners = (vx_scalar)parameters[TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryScalar(strength_thresh, (vx_enum)VX_SCALAR_TYPE, &strength_thresh_scalar_type, sizeof(strength_thresh_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(min_distance, (vx_enum)VX_SCALAR_TYPE, &min_distance_scalar_type, sizeof(min_distance_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(sensitivity, (vx_enum)VX_SCALAR_TYPE, &sensitivity_scalar_type, sizeof(sensitivity_scalar_type)));

        tivxCheckStatus(&status, vxQueryScalar(gradient_size, (vx_enum)VX_SCALAR_TYPE, &gradient_size_scalar_type, sizeof(gradient_size_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(gradient_size, &gradient_size_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryScalar(block_size, (vx_enum)VX_SCALAR_TYPE, &block_size_scalar_type, sizeof(block_size_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(block_size, &block_size_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryArray(corners, (vx_enum)VX_ARRAY_ITEMTYPE, &corners_item_type, sizeof(corners_item_type)));
        tivxCheckStatus(&status, vxQueryArray(corners, (vx_enum)VX_ARRAY_CAPACITY, &corners_capacity, sizeof(corners_capacity)));

        if (NULL != num_corners)
        {
            tivxCheckStatus(&status, vxQueryScalar(num_corners, (vx_enum)VX_SCALAR_TYPE, &num_corners_scalar_type, sizeof(num_corners_scalar_type)));
        }

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)corners);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8 != input_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_enum)VX_TYPE_FLOAT32 != strength_thresh_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'strength_thresh' should be a scalar of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((vx_enum)VX_TYPE_FLOAT32 != min_distance_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'min_distance' should be a scalar of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((vx_enum)VX_TYPE_FLOAT32 != sensitivity_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'sensitivity' should be a scalar of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((vx_enum)VX_TYPE_INT32 != gradient_size_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'gradient_size' should be a scalar of type:\n VX_TYPE_INT32 \n");
        }

        if ((vx_enum)VX_TYPE_INT32 != block_size_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'block_size' should be a scalar of type:\n VX_TYPE_INT32 \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_enum)VX_TYPE_KEYPOINT != corners_item_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'corners' should be an array of type:\n VX_TYPE_KEYPOINT \n");
            }
        }

        if (NULL != num_corners)
        {
            if ((vx_enum)VX_TYPE_SIZE != num_corners_scalar_type)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'num_corners' should be a scalar of type:\n VX_TYPE_SIZE \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (TIVX_KERNEL_HARRIS_CORNERS_MIN_SIZE  > input_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter 'input' should have a value for VX_IMAGE_WIDTH greater than TIVX_KERNEL_HARRIS_CORNERS_MIN_SIZE \n");
        }

        if (TIVX_KERNEL_HARRIS_CORNERS_MIN_SIZE  > input_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter 'input' should have a value for VX_IMAGE_HEIGHT greater than TIVX_KERNEL_HARRIS_CORNERS_MIN_SIZE \n");
        }

        if ((3 != gradient_size_val) &&
            (5 != gradient_size_val) &&
            (7 != gradient_size_val))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'gradient_size' should be a scalar with value:\n 3, 5, or 7 \n");
        }

        if ((3 != block_size_val) &&
            (5 != block_size_val) &&
            (7 != block_size_val))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'block_size' should be a scalar with value:\n 3, 5, or 7 \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED != border.mode)
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined border mode is supported for harris corners \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        corners_item_type = (vx_enum)VX_TYPE_KEYPOINT;

        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX], (vx_enum)VX_ARRAY_ITEMTYPE, &corners_item_type, sizeof(corners_item_type));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX], (vx_enum)VX_ARRAY_CAPACITY, &corners_capacity, sizeof(corners_capacity));

        if (NULL != num_corners)
        {
            vxSetMetaFormatAttribute(metas[TIVX_KERNEL_HARRIS_CORNERS_NUM_CORNERS_IDX], (vx_enum)VX_SCALAR_TYPE, &num_corners_scalar_type, sizeof(num_corners_scalar_type));
        }
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelHarrisCornersInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_STRENGTH_THRESH_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_MIN_DISTANCE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_SENSITIVITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_GRADIENT_SIZE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_BLOCK_SIZE_IDX])
        || (NULL == parameters[TIVX_KERNEL_HARRIS_CORNERS_CORNERS_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_HARRIS_CORNERS_INPUT_IDX];

        prms.num_input_images = 1U;
        prms.num_output_images = 0U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
    }

    return status;
}

vx_status tivxAddKernelHarrisCorners(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.harris_corners",
                (vx_enum)VX_KERNEL_HARRIS_CORNERS,
                NULL,
                TIVX_KERNEL_HARRIS_CORNERS_MAX_PARAMS,
                tivxAddKernelHarrisCornersValidate,
                tivxAddKernelHarrisCornersInitialize,
                NULL);

    status = vxGetStatus((vx_reference)kernel);

    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

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
                        (vx_enum)VX_TYPE_SCALAR,
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
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
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
    vx_harris_corners_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelHarrisCorners(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_harris_corners_kernel;

    status = vxRemoveKernel(kernel);
    vx_harris_corners_kernel = NULL;

    return status;
}


