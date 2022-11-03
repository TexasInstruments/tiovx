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
#include <tivx_kernel_remap.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_remap_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelRemapValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelRemapInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelRemapValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = NULL;
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_remap table = NULL;
    vx_uint32 table_src_w;
    vx_uint32 table_src_h;
    vx_uint32 table_dst_w;
    vx_uint32 table_dst_h;

    vx_scalar policy = NULL;
    vx_enum policy_scalar_type;
    vx_enum policy_val;

    vx_image output = NULL;
    vx_uint32 output_w;
    vx_uint32 output_h;
    vx_df_image output_fmt;

    vx_border_t border;

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_REMAP_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_REMAP_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_TABLE_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input = (vx_image)parameters[TIVX_KERNEL_REMAP_INPUT_IDX];
        table = (vx_remap)parameters[TIVX_KERNEL_REMAP_TABLE_IDX];
        policy = (vx_scalar)parameters[TIVX_KERNEL_REMAP_POLICY_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_REMAP_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));

        tivxCheckStatus(&status, vxQueryRemap(table, (vx_enum)VX_REMAP_SOURCE_WIDTH, &table_src_w, sizeof(table_src_w)));
        tivxCheckStatus(&status, vxQueryRemap(table, (vx_enum)VX_REMAP_SOURCE_HEIGHT, &table_src_h, sizeof(table_src_h)));
        tivxCheckStatus(&status, vxQueryRemap(table, (vx_enum)VX_REMAP_DESTINATION_WIDTH, &table_dst_w, sizeof(table_dst_w)));
        tivxCheckStatus(&status, vxQueryRemap(table, (vx_enum)VX_REMAP_DESTINATION_HEIGHT, &table_dst_h, sizeof(table_dst_h)));

        tivxCheckStatus(&status, vxQueryScalar(policy, (vx_enum)VX_SCALAR_TYPE, &policy_scalar_type, sizeof(policy_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(policy, &policy_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)output);

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

        if ((vx_enum)VX_TYPE_ENUM != policy_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'policy' should be a scalar of type:\n VX_TYPE_ENUM \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != output_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (input_w != table_src_w)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should have the same value for VX_IMAGE_WIDTH as 'table' does for VX_REMAP_SOURCE_WIDTH \n");
        }

        if (input_h != table_src_h)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input' should have the same value for VX_IMAGE_HEIGHT as 'table' does for VX_REMAP_SOURCE_HEIGHT \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if (output_w != table_dst_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output' should have the same value for VX_IMAGE_WIDTH as 'table' does for VX_REMAP_DESTINATION_WIDTH \n");
            }

            if (output_h != table_dst_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output' should have the same value for VX_IMAGE_HEIGHT as 'table' does for VX_REMAP_DESTINATION_HEIGHT \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_enum)VX_INTERPOLATION_NEAREST_NEIGHBOR != policy_val) &&
            ((vx_enum)VX_INTERPOLATION_BILINEAR != policy_val))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'policy' should be an enum of type:\n VX_INTERPOLATION_NEAREST_NEIGHBOR or VX_INTERPOLATION_BILINEAR \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_enum)VX_BORDER_UNDEFINED != border.mode) &&
            ((vx_enum)VX_BORDER_CONSTANT != border.mode))
        {
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            VX_PRINT(VX_ZONE_ERROR, "Only undefined and constant border mode is supported for remap \n");
        }
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_REMAP_OUTPUT_IDX], (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_REMAP_OUTPUT_IDX], (vx_enum)VX_IMAGE_WIDTH, &table_dst_w, sizeof(table_dst_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_REMAP_OUTPUT_IDX], (vx_enum)VX_IMAGE_HEIGHT, &table_dst_h, sizeof(table_dst_h));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelRemapInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_rectangle_t out_rect;

    vx_image output = NULL;
    vx_uint32 output_w;
    vx_uint32 output_h;

    if ( (num_params != TIVX_KERNEL_REMAP_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_REMAP_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_TABLE_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_POLICY_IDX])
        || (NULL == parameters[TIVX_KERNEL_REMAP_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        output = (vx_image)parameters[TIVX_KERNEL_REMAP_OUTPUT_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH, &output_w, sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h, sizeof(output_h)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        out_rect.start_x = 0U;
        out_rect.start_y = 0U;
        out_rect.end_x = output_w;
        out_rect.end_y = output_h;

        tivxCheckStatus(&status, vxSetImageValidRectangle(output, &out_rect));
    }

    return status;
}

vx_status tivxAddKernelRemap(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.remap",
                (vx_enum)VX_KERNEL_REMAP,
                NULL,
                TIVX_KERNEL_REMAP_MAX_PARAMS,
                tivxAddKernelRemapValidate,
                tivxAddKernelRemapInitialize,
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
                        (vx_enum)VX_TYPE_REMAP,
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
    vx_remap_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelRemap(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_remap_kernel;

    status = vxRemoveKernel(kernel);
    vx_remap_kernel = NULL;

    return status;
}


