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
#include <tivx_kernel_multiply.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_multiply_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelMultiplyValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelMultiplyInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelMultiplyValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image in1 = NULL;
    vx_uint32 in1_w;
    vx_uint32 in1_h;
    vx_df_image in1_fmt;

    vx_image in2 = NULL;
    vx_uint32 in2_w;
    vx_uint32 in2_h;
    vx_df_image in2_fmt;

    vx_scalar scale = NULL;
    vx_enum scale_scalar_type;
    vx_float32 scale_val;

    vx_scalar overflow_policy = NULL;
    vx_enum overflow_policy_scalar_type;
    vx_enum overflow_policy_val;

    vx_scalar rounding_policy = NULL;
    vx_enum rounding_policy_scalar_type;
    vx_enum rounding_policy_val;

    vx_image out = NULL;
    vx_uint32 out_w;
    vx_uint32 out_h;
    vx_df_image out_fmt;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_MULTIPLY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_IN1_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_IN2_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_SCALE_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_OVERFLOW_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_ROUNDING_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_OUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        in1 = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_IN1_IDX];
        in2 = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_IN2_IDX];
        scale = (vx_scalar)parameters[TIVX_KERNEL_MULTIPLY_SCALE_IDX];
        overflow_policy = (vx_scalar)parameters[TIVX_KERNEL_MULTIPLY_OVERFLOW_POLICY_IDX];
        rounding_policy = (vx_scalar)parameters[TIVX_KERNEL_MULTIPLY_ROUNDING_POLICY_IDX];
        out = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_OUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(in1, (vx_enum)VX_IMAGE_FORMAT, &in1_fmt, sizeof(in1_fmt)));
        tivxCheckStatus(&status, vxQueryImage(in1, (vx_enum)VX_IMAGE_WIDTH, &in1_w, sizeof(in1_w)));
        tivxCheckStatus(&status, vxQueryImage(in1, (vx_enum)VX_IMAGE_HEIGHT, &in1_h, sizeof(in1_h)));

        tivxCheckStatus(&status, vxQueryImage(in2, (vx_enum)VX_IMAGE_FORMAT, &in2_fmt, sizeof(in2_fmt)));
        tivxCheckStatus(&status, vxQueryImage(in2, (vx_enum)VX_IMAGE_WIDTH, &in2_w, sizeof(in2_w)));
        tivxCheckStatus(&status, vxQueryImage(in2, (vx_enum)VX_IMAGE_HEIGHT, &in2_h, sizeof(in2_h)));

        tivxCheckStatus(&status, vxQueryScalar(scale, (vx_enum)VX_SCALAR_TYPE, &scale_scalar_type, sizeof(scale_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(scale, &scale_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryScalar(overflow_policy, (vx_enum)VX_SCALAR_TYPE, &overflow_policy_scalar_type, sizeof(overflow_policy_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(overflow_policy, &overflow_policy_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryScalar(rounding_policy, (vx_enum)VX_SCALAR_TYPE, &rounding_policy_scalar_type, sizeof(rounding_policy_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(rounding_policy, &rounding_policy_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryImage(out, (vx_enum)VX_IMAGE_FORMAT, &out_fmt, sizeof(out_fmt)));
        tivxCheckStatus(&status, vxQueryImage(out, (vx_enum)VX_IMAGE_WIDTH, &out_w, sizeof(out_w)));
        tivxCheckStatus(&status, vxQueryImage(out, (vx_enum)VX_IMAGE_HEIGHT, &out_h, sizeof(out_h)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)out);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_U8 != in1_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_S16 != in1_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in1' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_S16 \n");
        }

        if (((vx_df_image)VX_DF_IMAGE_U8 != in2_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_S16 != in2_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_S16 \n");
        }

        if ((vx_enum)VX_TYPE_FLOAT32 != scale_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'scale' should be a scalar of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((vx_enum)VX_TYPE_ENUM != overflow_policy_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'overflow_policy' should be a scalar of type:\n VX_TYPE_ENUM \n");
        }

        if ((vx_enum)VX_TYPE_ENUM != rounding_policy_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'rounding_policy' should be a scalar of type:\n VX_TYPE_ENUM \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (((vx_df_image)VX_DF_IMAGE_U8 != out_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_S16 != out_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_S16 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (in1_w != in2_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'in1' and 'in2' should have the same value for VX_IMAGE_WIDTH \n");
        }

        if (in1_h != in2_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'in1' and 'in2' should have the same value for VX_IMAGE_HEIGHT \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (((vx_df_image)VX_DF_IMAGE_U8 == out_fmt) &&
                (((vx_df_image)VX_DF_IMAGE_S16 == in1_fmt) ||
                ((vx_df_image)VX_DF_IMAGE_S16 == in2_fmt)))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out' must not be VX_DF_IMAGE_U8 if 'in1' or 'in2' are VX_DF_IMAGE_S16 \n");
            }

            if (in1_w != out_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'in1' and 'out' should have the same value for VX_IMAGE_WIDTH \n");
            }

            if (in1_h != out_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'in1' and 'out' should have the same value for VX_IMAGE_HEIGHT \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (0.0 > scale_val)
        {
            status = (vx_status)VX_ERROR_INVALID_VALUE;
            VX_PRINT(VX_ZONE_ERROR, "'scale' must be greater than or equal to 0 \n");
        }

        if (((vx_enum)VX_CONVERT_POLICY_WRAP != overflow_policy_val) &&
            ((vx_enum)VX_CONVERT_POLICY_SATURATE != overflow_policy_val))
        {
            status = (vx_status)VX_ERROR_INVALID_VALUE;
            VX_PRINT(VX_ZONE_ERROR, "'overflow_policy' value should be:\n VX_CONVERT_POLICY_WRAP or VX_CONVERT_POLICY_SATURATE \n");
        }

        if (((vx_enum)VX_ROUND_POLICY_TO_ZERO != rounding_policy_val) &&
            ((vx_enum)VX_ROUND_POLICY_TO_NEAREST_EVEN != rounding_policy_val))
        {
            status = (vx_status)VX_ERROR_INVALID_VALUE;
            VX_PRINT(VX_ZONE_ERROR, "'rounding_policy' value should be:\n VX_ROUND_POLICY_TO_ZERO or VX_ROUND_POLICY_TO_NEAREST_EVEN \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_S16 == in1_fmt) ||
            (((vx_df_image)VX_DF_IMAGE_S16 == in2_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_U8 != out_fmt)))
        {
            out_fmt = (vx_df_image)VX_DF_IMAGE_S16;
        }
        else
        {
            out_fmt = (vx_df_image)VX_DF_IMAGE_U8;
        }

        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MULTIPLY_OUT_IDX], (vx_enum)VX_IMAGE_FORMAT, &out_fmt, sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MULTIPLY_OUT_IDX], (vx_enum)VX_IMAGE_WIDTH, &in1_w, sizeof(in1_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_MULTIPLY_OUT_IDX], (vx_enum)VX_IMAGE_HEIGHT, &in1_h, sizeof(in1_h));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelMultiplyInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_MULTIPLY_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_IN1_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_IN2_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_SCALE_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_OVERFLOW_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_ROUNDING_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_MULTIPLY_OUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_IN1_IDX];
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_IN2_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_MULTIPLY_OUT_IDX];

        prms.num_input_images = 2U;
        prms.num_output_images = 1U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
    }

    return status;
}

vx_status tivxAddKernelMultiply(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.multiply",
                (vx_enum)VX_KERNEL_MULTIPLY,
                NULL,
                TIVX_KERNEL_MULTIPLY_MAX_PARAMS,
                tivxAddKernelMultiplyValidate,
                tivxAddKernelMultiplyInitialize,
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
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
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
    vx_multiply_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelMultiply(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_multiply_kernel;

    status = vxRemoveKernel(kernel);
    vx_multiply_kernel = NULL;

    return status;
}


