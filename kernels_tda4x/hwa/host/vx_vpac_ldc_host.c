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
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_vpac_ldc_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVpacLdcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVpacLdcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelVpacLdc(vx_context context);
vx_status tivxRemoveKernelVpacLdc(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelVpacLdcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_array configuration = NULL;
    vx_enum configuration_item_type;
    vx_size configuration_capacity, configuration_item_size;

    vx_array region_params = NULL;
    vx_enum region_params_item_type;
    vx_size region_params_capacity, region_params_item_size;

    vx_image mesh_table = NULL;
    vx_df_image mesh_table_fmt;
    vx_uint32 mesh_table_w, mesh_table_h;

    vx_matrix warp_matrix = NULL;
    vx_enum warp_matrix_type;
    vx_size warp_matrix_w, warp_matrix_h;

    vx_lut out_2_luma_lut = NULL;
    vx_enum out_2_luma_lut_type;
    vx_size out_2_luma_lut_count;

    vx_lut out_3_chroma_lut = NULL;
    vx_enum out_3_chroma_lut_type;
    vx_size out_3_chroma_lut_count;

    vx_array bandwidth_params = NULL;
    vx_enum bandwidth_params_item_type;
    vx_size bandwidth_params_capacity, bandwidth_params_item_size;

    vx_image in_luma_or_422 = NULL;
    vx_df_image in_luma_or_422_fmt;
    vx_uint32 in_luma_or_422_w, in_luma_or_422_h;

    vx_image in_chroma = NULL;
    vx_df_image in_chroma_fmt;
    vx_uint32 in_chroma_w, in_chroma_h;

    vx_image out_0_luma_or_422 = NULL;
    vx_df_image out_0_luma_or_422_fmt;
    vx_uint32 out_0_luma_or_422_w, out_0_luma_or_422_h;

    vx_image out_1_chroma = NULL;
    vx_df_image out_1_chroma_fmt;
    vx_uint32 out_1_chroma_w, out_1_chroma_h;

    vx_image out_2_luma_or_422 = NULL;
    vx_df_image out_2_luma_or_422_fmt;
    vx_uint32 out_2_luma_or_422_w, out_2_luma_or_422_h;

    vx_image out_3_chroma = NULL;
    vx_df_image out_3_chroma_fmt;
    vx_uint32 out_3_chroma_w, out_3_chroma_h;

    vx_scalar error_status = NULL;
    vx_enum error_status_scalar_type;

    if ( (num != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (const vx_array)parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
        region_params = (const vx_array)parameters[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX];
        mesh_table = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_MESH_TABLE_IDX];
        warp_matrix = (const vx_matrix)parameters[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
        out_2_luma_lut = (const vx_lut)parameters[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_LUT_IDX];
        out_3_chroma_lut = (const vx_lut)parameters[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_LUT_IDX];
        bandwidth_params = (const vx_array)parameters[TIVX_KERNEL_VPAC_LDC_BANDWIDTH_PARAMS_IDX];
        in_luma_or_422 = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX];
        in_chroma = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX];
        out_0_luma_or_422 = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX];
        out_1_chroma = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX];
        out_2_luma_or_422 = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_OR_422_IDX];
        out_3_chroma = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_IDX];
        error_status = (const vx_scalar)parameters[TIVX_KERNEL_VPAC_LDC_ERROR_STATUS_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMTYPE, &configuration_item_type, sizeof(configuration_item_type)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_CAPACITY, &configuration_capacity, sizeof(configuration_capacity)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMSIZE, &configuration_item_size, sizeof(configuration_item_size)));

        tivxCheckStatus(&status, vxQueryArray(region_params, VX_ARRAY_ITEMTYPE, &region_params_item_type, sizeof(region_params_item_type)));
        tivxCheckStatus(&status, vxQueryArray(region_params, VX_ARRAY_CAPACITY, &region_params_capacity, sizeof(region_params_capacity)));
        tivxCheckStatus(&status, vxQueryArray(region_params, VX_ARRAY_ITEMSIZE, &region_params_item_size, sizeof(region_params_item_size)));

        if (NULL != mesh_table)
        {
            tivxCheckStatus(&status, vxQueryImage(mesh_table, VX_IMAGE_FORMAT, &mesh_table_fmt, sizeof(mesh_table_fmt)));
            tivxCheckStatus(&status, vxQueryImage(mesh_table, VX_IMAGE_WIDTH, &mesh_table_w, sizeof(mesh_table_w)));
            tivxCheckStatus(&status, vxQueryImage(mesh_table, VX_IMAGE_HEIGHT, &mesh_table_h, sizeof(mesh_table_h)));
        }

        if (NULL != warp_matrix)
        {
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, VX_MATRIX_TYPE, &warp_matrix_type, sizeof(warp_matrix_type)));
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, VX_MATRIX_COLUMNS, &warp_matrix_w, sizeof(warp_matrix_w)));
            tivxCheckStatus(&status, vxQueryMatrix(warp_matrix, VX_MATRIX_ROWS, &warp_matrix_h, sizeof(warp_matrix_h)));
        }

        if (NULL != out_2_luma_lut)
        {
            tivxCheckStatus(&status, vxQueryLUT(out_2_luma_lut, VX_LUT_TYPE, &out_2_luma_lut_type, sizeof(out_2_luma_lut_type)));
            tivxCheckStatus(&status, vxQueryLUT(out_2_luma_lut, VX_LUT_COUNT, &out_2_luma_lut_count, sizeof(out_2_luma_lut_count)));
        }

        if (NULL != out_3_chroma_lut)
        {
            tivxCheckStatus(&status, vxQueryLUT(out_3_chroma_lut, VX_LUT_TYPE, &out_3_chroma_lut_type, sizeof(out_3_chroma_lut_type)));
            tivxCheckStatus(&status, vxQueryLUT(out_3_chroma_lut, VX_LUT_COUNT, &out_3_chroma_lut_count, sizeof(out_3_chroma_lut_count)));
        }

        if (NULL != bandwidth_params)
        {
            tivxCheckStatus(&status, vxQueryArray(bandwidth_params, VX_ARRAY_ITEMTYPE, &bandwidth_params_item_type, sizeof(bandwidth_params_item_type)));
            tivxCheckStatus(&status, vxQueryArray(bandwidth_params, VX_ARRAY_CAPACITY, &bandwidth_params_capacity, sizeof(bandwidth_params_capacity)));
            tivxCheckStatus(&status, vxQueryArray(bandwidth_params, VX_ARRAY_ITEMSIZE, &bandwidth_params_item_size, sizeof(bandwidth_params_item_size)));
        }

        if (NULL != in_luma_or_422)
        {
            tivxCheckStatus(&status, vxQueryImage(in_luma_or_422, VX_IMAGE_FORMAT, &in_luma_or_422_fmt, sizeof(in_luma_or_422_fmt)));
            tivxCheckStatus(&status, vxQueryImage(in_luma_or_422, VX_IMAGE_WIDTH, &in_luma_or_422_w, sizeof(in_luma_or_422_w)));
            tivxCheckStatus(&status, vxQueryImage(in_luma_or_422, VX_IMAGE_HEIGHT, &in_luma_or_422_h, sizeof(in_luma_or_422_h)));
        }

        if (NULL != in_chroma)
        {
            tivxCheckStatus(&status, vxQueryImage(in_chroma, VX_IMAGE_FORMAT, &in_chroma_fmt, sizeof(in_chroma_fmt)));
            tivxCheckStatus(&status, vxQueryImage(in_chroma, VX_IMAGE_WIDTH, &in_chroma_w, sizeof(in_chroma_w)));
            tivxCheckStatus(&status, vxQueryImage(in_chroma, VX_IMAGE_HEIGHT, &in_chroma_h, sizeof(in_chroma_h)));
        }

        if (NULL != out_0_luma_or_422)
        {
            tivxCheckStatus(&status, vxQueryImage(out_0_luma_or_422, VX_IMAGE_FORMAT, &out_0_luma_or_422_fmt, sizeof(out_0_luma_or_422_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out_0_luma_or_422, VX_IMAGE_WIDTH, &out_0_luma_or_422_w, sizeof(out_0_luma_or_422_w)));
            tivxCheckStatus(&status, vxQueryImage(out_0_luma_or_422, VX_IMAGE_HEIGHT, &out_0_luma_or_422_h, sizeof(out_0_luma_or_422_h)));
        }

        if (NULL != out_1_chroma)
        {
            tivxCheckStatus(&status, vxQueryImage(out_1_chroma, VX_IMAGE_FORMAT, &out_1_chroma_fmt, sizeof(out_1_chroma_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out_1_chroma, VX_IMAGE_WIDTH, &out_1_chroma_w, sizeof(out_1_chroma_w)));
            tivxCheckStatus(&status, vxQueryImage(out_1_chroma, VX_IMAGE_HEIGHT, &out_1_chroma_h, sizeof(out_1_chroma_h)));
        }

        if (NULL != out_2_luma_or_422)
        {
            tivxCheckStatus(&status, vxQueryImage(out_2_luma_or_422, VX_IMAGE_FORMAT, &out_2_luma_or_422_fmt, sizeof(out_2_luma_or_422_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out_2_luma_or_422, VX_IMAGE_WIDTH, &out_2_luma_or_422_w, sizeof(out_2_luma_or_422_w)));
            tivxCheckStatus(&status, vxQueryImage(out_2_luma_or_422, VX_IMAGE_HEIGHT, &out_2_luma_or_422_h, sizeof(out_2_luma_or_422_h)));
        }

        if (NULL != out_3_chroma)
        {
            tivxCheckStatus(&status, vxQueryImage(out_3_chroma, VX_IMAGE_FORMAT, &out_3_chroma_fmt, sizeof(out_3_chroma_fmt)));
            tivxCheckStatus(&status, vxQueryImage(out_3_chroma, VX_IMAGE_WIDTH, &out_3_chroma_w, sizeof(out_3_chroma_w)));
            tivxCheckStatus(&status, vxQueryImage(out_3_chroma, VX_IMAGE_HEIGHT, &out_3_chroma_h, sizeof(out_3_chroma_h)));
        }

        if (NULL != error_status)
        {
            tivxCheckStatus(&status, vxQueryScalar(error_status, VX_SCALAR_TYPE, &error_status_scalar_type, sizeof(error_status_scalar_type)));
        }
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ( configuration_item_size != sizeof(tivx_vpac_ldc_params_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an array of type:\n tivx_vpac_ldc_params_t \n");
        }

        if( (region_params_item_size != sizeof(tivx_vpac_ldc_region_params_t)) &&
            (region_params_item_size != sizeof(tivx_vpac_ldc_subregion_params_t)))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'region_params' should be an array of type:\n tivx_vpac_ldc_region_params_t or tivx_vpac_ldc_subregion_params_t \n");
        }

        if (NULL != mesh_table)
        {
            if (VX_DF_IMAGE_U32 != mesh_table_fmt)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'mesh_table' should be an image of type:\n VX_DF_IMAGE_U32 \n");
            }
        }

        if (NULL != warp_matrix)
        {
            if( (VX_TYPE_INT16 != warp_matrix_type) &&
                (VX_TYPE_FLOAT32 != warp_matrix_type))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'warp_matrix' should be a matrix of type:\n VX_TYPE_INT16 or VX_TYPE_FLOAT32 \n");
            }
        }

        if (NULL != out_2_luma_lut)
        {
            if (VX_TYPE_UINT16 != out_2_luma_lut_type)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_2_luma_lut' should be a lut of type:\n VX_TYPE_UINT16 \n");
            }
        }

        if (NULL != out_3_chroma_lut)
        {
            if (VX_TYPE_UINT16 != out_3_chroma_lut_type)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_3_chroma_lut' should be a lut of type:\n VX_TYPE_UINT16 \n");
            }
        }

        if (NULL != bandwidth_params)
        {
            if ( bandwidth_params_item_size != sizeof(tivx_vpac_ldc_bandwidth_params_t))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'bandwidth_params' should be an array of type:\n tivx_vpac_ldc_bandwidth_params_t \n");
            }
        }

        if (NULL != in_luma_or_422)
        {
            if( (VX_DF_IMAGE_UYVY != in_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U8 != in_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U16 != in_luma_or_422_fmt) &&
                (TIVX_DF_IMAGE_P12 != in_luma_or_422_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_luma_or_422' should be an image of type:\n VX_DF_IMAGE_UYVY or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != in_chroma)
        {
            if( (VX_DF_IMAGE_U8 != in_chroma_fmt) &&
                (VX_DF_IMAGE_U16 != in_chroma_fmt) &&
                (TIVX_DF_IMAGE_P12 != in_chroma_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_chroma' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != out_0_luma_or_422)
        {
            if( (VX_DF_IMAGE_UYVY != out_0_luma_or_422_fmt) &&
                (VX_DF_IMAGE_YUYV != out_0_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U8 != out_0_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U16 != out_0_luma_or_422_fmt) &&
                (TIVX_DF_IMAGE_P12 != out_0_luma_or_422_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_0_luma_or_422' should be an image of type:\n VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != out_1_chroma)
        {
            if( (VX_DF_IMAGE_U8 != out_1_chroma_fmt) &&
                (VX_DF_IMAGE_U16 != out_1_chroma_fmt) &&
                (TIVX_DF_IMAGE_P12 != out_1_chroma_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_1_chroma' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != out_2_luma_or_422)
        {
            if( (VX_DF_IMAGE_UYVY != out_2_luma_or_422_fmt) &&
                (VX_DF_IMAGE_YUYV != out_2_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U8 != out_2_luma_or_422_fmt) &&
                (VX_DF_IMAGE_U16 != out_2_luma_or_422_fmt) &&
                (TIVX_DF_IMAGE_P12 != out_2_luma_or_422_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_2_luma_or_422' should be an image of type:\n VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != out_3_chroma)
        {
            if( (VX_DF_IMAGE_U8 != out_3_chroma_fmt) &&
                (VX_DF_IMAGE_U16 != out_3_chroma_fmt) &&
                (TIVX_DF_IMAGE_P12 != out_3_chroma_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_3_chroma' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != error_status)
        {
            if (VX_TYPE_UINT32 != error_status_scalar_type)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'error_status' should be a scalar of type:\n VX_TYPE_UINT32 \n");
            }
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    if (VX_SUCCESS == status)
    {
        if(NULL != warp_matrix)
        {
            if ((2 != warp_matrix_w) && (3 != warp_matrix_w))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'warp_matrix' columns should be 2 (affine warp) or 3 (perspective warp) \n");
            }
            if (3 != warp_matrix_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'warp_matrix' rows should be 3 \n");
            }
        }

        if (NULL != out_2_luma_lut)
        {
            if (513 != out_2_luma_lut_count)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_2_luma_lut' count should be 513 \n");
            }
        }

        if (NULL != out_3_chroma_lut)
        {
            if (513 != out_3_chroma_lut_count)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'out_3_chroma_lut' count should be 513 \n");
		    }
        }
    }

    /* Check input formats and relative sizes */
    if (VX_SUCCESS == status)
    {
        if (NULL != in_luma_or_422)
        {
            if(VX_DF_IMAGE_UYVY == in_luma_or_422_fmt)
            {
                if (NULL != in_chroma)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "If 'in_luma_or_422' is format VX_DF_IMAGE_UYVY, then in_chroma should be NULL.\n");
                }
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if ((NULL != in_luma_or_422) && (NULL != in_chroma))
        {
            if ((in_luma_or_422_w != in_chroma_w) || (in_luma_or_422_h != (in_chroma_h*2)))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Input image sizes do not match (widths should be same, in_chroma height should be 1/2 of luma height \n");
                VX_PRINT(VX_ZONE_ERROR, "in_luma_or_422 width = %d\n", in_luma_or_422_w);
                VX_PRINT(VX_ZONE_ERROR, "in_luma_or_422 height = %d\n", in_luma_or_422_h);
                VX_PRINT(VX_ZONE_ERROR, "in_chroma width = %d\n", in_chroma_w);
                VX_PRINT(VX_ZONE_ERROR, "in_chroma height = %d\n", in_chroma_h);
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        if ((NULL == in_luma_or_422) && (NULL == in_chroma))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_luma_or_422' and 'in_chroma' should not BOTH be NULL.  At least one should be non-NULL \n");
        }
    }

    /* Check output formats and relative sizes */
    if (VX_SUCCESS == status)
    {
        if (NULL != out_0_luma_or_422)
        {
            if((VX_DF_IMAGE_UYVY == out_0_luma_or_422_fmt) || (VX_DF_IMAGE_YUYV == out_0_luma_or_422_fmt))
            {
                if (NULL != out_1_chroma)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "If 'out_0_luma_or_422' is format VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV, then out_1_chroma should be NULL.\n");
                }
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        if ((NULL != out_0_luma_or_422) && (NULL != out_1_chroma))
        {
            if ((out_0_luma_or_422_w != out_1_chroma_w) ||
                 ((out_0_luma_or_422_h != (out_1_chroma_h*2)) && (out_0_luma_or_422_h != out_1_chroma_h)))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Output image sizes do not match (widths should be same, out_1_chroma should be 1/2 or equal to out_0_luma height \n");
                VX_PRINT(VX_ZONE_ERROR, "out_0_luma_or_422 width = %d\n", out_0_luma_or_422_w);
                VX_PRINT(VX_ZONE_ERROR, "out_0_luma_or_422 height = %d\n", out_0_luma_or_422_h);
                VX_PRINT(VX_ZONE_ERROR, "out_1_chroma width = %d\n", out_1_chroma_w);
                VX_PRINT(VX_ZONE_ERROR, "out_1_chroma height = %d\n", out_1_chroma_h);
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        if (NULL != out_2_luma_or_422)
        {
            if((VX_DF_IMAGE_UYVY == out_2_luma_or_422_fmt) || (VX_DF_IMAGE_YUYV == out_2_luma_or_422_fmt))
            {
                if (NULL != out_3_chroma)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "If 'out_2_luma_or_422' is format VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV, then out_3_chroma should be NULL.\n");
                }
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        if ((NULL != out_2_luma_or_422) && (NULL != out_3_chroma))
        {
            if ((out_2_luma_or_422_w != out_3_chroma_w) ||
                 ((out_2_luma_or_422_h != (out_3_chroma_h*2)) && (out_2_luma_or_422_h != out_3_chroma_h)))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Output image sizes do not match (widths should be same, out_3_chroma should be 1/2 or equal to out_2_luma height \n");
                VX_PRINT(VX_ZONE_ERROR, "out_2_luma_or_422 width = %d\n", out_2_luma_or_422_w);
                VX_PRINT(VX_ZONE_ERROR, "out_2_luma_or_422 height = %d\n", out_2_luma_or_422_h);
                VX_PRINT(VX_ZONE_ERROR, "out_3_chroma width = %d\n", out_3_chroma_w);
                VX_PRINT(VX_ZONE_ERROR, "out_3_chroma height = %d\n", out_3_chroma_h);
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        if ((NULL == out_0_luma_or_422) && (NULL == out_1_chroma) && (NULL == out_2_luma_or_422) && (NULL == out_3_chroma))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "At least one output should be non-NULL \n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacLdcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        if(NULL != parameters[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX])
        {
            prms.in_img[0U] = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX];
        }
        else if(NULL != parameters[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX])
        {
            prms.in_img[0U] = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX];
        }
        else
        {
            /* Do nothing */
        }

        if(NULL != parameters[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX])
        {
            prms.out_img[0U] = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX];
        }
        else if(NULL != parameters[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX])
        {
            prms.out_img[0U] = (const vx_image)parameters[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX];
        }
        else
        {
            /* Do nothing */
        }

        prms.num_input_images = 1;
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

vx_status tivxAddKernelVpacLdc(vx_context context)
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
                    TIVX_KERNEL_VPAC_LDC_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_LDC_MAX_PARAMS,
                    tivxAddKernelVpacLdcValidate,
                    tivxAddKernelVpacLdcInitialize,
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
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_MATRIX,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_LUT,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_LUT,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_ARRAY,
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
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_UINT32,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_LDC1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_LDC2);
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
    vx_vpac_ldc_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacLdc(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_ldc_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_ldc_kernel = NULL;

    return status;
}


