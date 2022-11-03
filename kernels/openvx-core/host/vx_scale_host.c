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
#include <tivx_kernel_scale.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_core_host_priv.h"

static vx_kernel vx_scale_image_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelScaleValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelScaleInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelScaleValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image src = NULL;
    vx_uint32 src_w;
    vx_uint32 src_h;
    vx_df_image src_fmt;

    vx_image dst = NULL;
    vx_uint32 dst_w;
    vx_uint32 dst_h;
    vx_df_image dst_fmt;

    vx_scalar type = NULL;
    vx_enum type_scalar_type;
    vx_enum type_val;

    char node_target[TIVX_TARGET_MAX_NAME];

    vx_bool is_virtual = (vx_bool)vx_false_e;

    if ( (num != TIVX_KERNEL_SCALE_IMAGE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_SRC_IDX])
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_DST_IDX])
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_TYPE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (vx_image)parameters[TIVX_KERNEL_SCALE_IMAGE_SRC_IDX];
        dst = (vx_image)parameters[TIVX_KERNEL_SCALE_IMAGE_DST_IDX];
        type = (vx_scalar)parameters[TIVX_KERNEL_SCALE_IMAGE_TYPE_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(src, (vx_enum)VX_IMAGE_FORMAT, &src_fmt, sizeof(src_fmt)));
        tivxCheckStatus(&status, vxQueryImage(src, (vx_enum)VX_IMAGE_WIDTH, &src_w, sizeof(src_w)));
        tivxCheckStatus(&status, vxQueryImage(src, (vx_enum)VX_IMAGE_HEIGHT, &src_h, sizeof(src_h)));

        tivxCheckStatus(&status, vxQueryImage(dst, (vx_enum)VX_IMAGE_FORMAT, &dst_fmt, sizeof(dst_fmt)));
        tivxCheckStatus(&status, vxQueryImage(dst, (vx_enum)VX_IMAGE_WIDTH, &dst_w, sizeof(dst_w)));
        tivxCheckStatus(&status, vxQueryImage(dst, (vx_enum)VX_IMAGE_HEIGHT, &dst_h, sizeof(dst_h)));

        tivxCheckStatus(&status, vxQueryScalar(type, (vx_enum)VX_SCALAR_TYPE, &type_scalar_type, sizeof(type_scalar_type)));
        tivxCheckStatus(&status, vxCopyScalar(type, &type_val, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST));

        tivxCheckStatus(&status, vxQueryNode(node, (vx_enum)TIVX_NODE_TARGET_STRING, &node_target, sizeof(node_target)));

#if 1

        is_virtual = tivxIsReferenceVirtual((vx_reference)dst);

#endif

    }


    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_df_image)VX_DF_IMAGE_U8 != src_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'src' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((vx_bool)vx_false_e == is_virtual)
        {
            if ((vx_df_image)VX_DF_IMAGE_U8 != dst_fmt)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'dst' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
        }

        if ((vx_enum)VX_TYPE_ENUM != type_scalar_type)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'type' should be a scalar of type:\n VX_TYPE_ENUM \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_enum)VX_INTERPOLATION_NEAREST_NEIGHBOR != type_val) &&
            ((vx_enum)VX_INTERPOLATION_BILINEAR != type_val) &&
            ((vx_enum)VX_INTERPOLATION_AREA != type_val))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'type' value should be an enum of type:\n VX_INTERPOLATION_NEAREST_NEIGHBOR or VX_INTERPOLATION_BILINEAR or VX_INTERPOLATION_AREA \n");
        }

#if !defined(SOC_J6)
        if ((0 == strncmp(node_target, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME)) ||
            (0 == strncmp(node_target, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_MAX_NAME))
#if defined (SOC_J784S4)
            || (0 == strncmp(node_target, TIVX_TARGET_VPAC2_MSC1, TIVX_TARGET_MAX_NAME)) ||
               (0 == strncmp(node_target, TIVX_TARGET_VPAC2_MSC2, TIVX_TARGET_MAX_NAME))
#endif
            )
        {
            if ((vx_bool)vx_false_e == is_virtual)
            {
                if (dst_w > src_w)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'src' width must be greater than 'dst' width \n");
                }

                if (dst_h > src_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'src' height must be greater than 'dst' height \n");
                }

                if ((4U * dst_w) < src_w)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'src' width must not be greater than 'dst' width * 4 \n");
                }

                if ((4U * dst_h) < src_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'src' height must not be greater than 'dst' height * 4 \n");
                }
            }
        }
#endif
    }

#if 1

    if ((vx_status)VX_SUCCESS == status)
    {
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SCALE_IMAGE_DST_IDX], (vx_enum)VX_IMAGE_FORMAT, &src_fmt, sizeof(src_fmt));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SCALE_IMAGE_DST_IDX], (vx_enum)VX_IMAGE_WIDTH, &dst_w, sizeof(dst_w));
        vxSetMetaFormatAttribute(metas[TIVX_KERNEL_SCALE_IMAGE_DST_IDX], (vx_enum)VX_IMAGE_HEIGHT, &dst_h, sizeof(dst_h));
    }

#endif

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelScaleInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    vx_image src = NULL;
    vx_uint32 src_w;
    vx_uint32 src_h;

    vx_image dst = NULL;
    vx_uint32 dst_w;
    vx_uint32 dst_h;

    vx_rectangle_t rect;
    vx_rectangle_t out_rect;
    vx_uint32 scale;

    if ( (num_params != TIVX_KERNEL_SCALE_IMAGE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_SRC_IDX])
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_DST_IDX])
        || (NULL == parameters[TIVX_KERNEL_SCALE_IMAGE_TYPE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (vx_image)parameters[TIVX_KERNEL_SCALE_IMAGE_SRC_IDX];
        dst = (vx_image)parameters[TIVX_KERNEL_SCALE_IMAGE_DST_IDX];
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(src, (vx_enum)VX_IMAGE_WIDTH, &src_w, sizeof(src_w)));
        tivxCheckStatus(&status, vxQueryImage(src, (vx_enum)VX_IMAGE_HEIGHT, &src_h, sizeof(src_h)));

        tivxCheckStatus(&status, vxQueryImage(dst, (vx_enum)VX_IMAGE_WIDTH, &dst_w, sizeof(dst_w)));
        tivxCheckStatus(&status, vxQueryImage(dst, (vx_enum)VX_IMAGE_HEIGHT, &dst_h, sizeof(dst_h)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0] = src;
        prms.out_img[0] = dst;

        prms.num_input_images = 1U;
        prms.num_output_images = 1U;

        prms.top_pad = 0U;
        prms.bot_pad = 0U;
        prms.left_pad = 0U;
        prms.right_pad = 0U;
        prms.border_mode = (vx_enum)VX_BORDER_UNDEFINED;

        tivxCheckStatus(&status, tivxKernelConfigValidRect(&prms));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (dst_w > src_w)
        {
            scale = dst_w / src_w;
            tivxCheckStatus(&status, vxGetValidRegionImage(src, &rect));

            out_rect.start_x = rect.start_x;
            out_rect.start_y = rect.start_y;

            if (1U == (dst_w - src_w))
            {
                out_rect.end_x = rect.end_x + 1U;
                out_rect.end_y = rect.end_y + 1U;
            }
            else
            {
                out_rect.end_x = rect.end_x * scale;
                out_rect.end_y = rect.end_y * scale;
            }

            tivxCheckStatus(&status, vxSetImageValidRectangle(dst, &out_rect));
        }
    }

    return status;
}

vx_status tivxAddKernelScale(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;

    kernel = vxAddUserKernel(
                context,
                "org.khronos.openvx.scale_image",
                (vx_enum)VX_KERNEL_SCALE_IMAGE,
                NULL,
                TIVX_KERNEL_SCALE_IMAGE_MAX_PARAMS,
                tivxAddKernelScaleValidate,
                tivxAddKernelScaleInitialize,
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
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_SCALAR,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
#if !defined(SOC_J6)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2);
#if defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2);
#endif
#endif
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
    vx_scale_image_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelScale(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_scale_image_kernel;

    status = vxRemoveKernel(kernel);
    vx_scale_image_kernel = NULL;

    return status;
}


