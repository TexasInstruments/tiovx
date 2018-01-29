/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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
#include "tivx_kernel_vpac_viss.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_vpac_viss_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVpacVissValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVpacVissInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelVpacViss(vx_context context);
vx_status tivxRemoveKernelVpacViss(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelVpacVissValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_array configuration = NULL;
    vx_enum configuration_item_type;
    vx_size configuration_capacity, configuration_item_size;

    vx_array ae_awb_result = NULL;
    vx_enum ae_awb_result_item_type;
    vx_size ae_awb_result_capacity, ae_awb_result_item_size;

    vx_image raw0 = NULL;
    vx_df_image raw0_fmt;
    vx_uint32 raw0_w, raw0_h;

    vx_image raw1 = NULL;
    vx_df_image raw1_fmt;
    vx_uint32 raw1_w, raw1_h;

    vx_image raw2 = NULL;
    vx_df_image raw2_fmt;
    vx_uint32 raw2_w, raw2_h;

    vx_image y12 = NULL;
    vx_df_image y12_fmt;
    vx_uint32 y12_w, y12_h;

    vx_image uv12_c1 = NULL;
    vx_df_image uv12_c1_fmt;
    vx_uint32 uv12_c1_w, uv12_c1_h;

    vx_image y8_r8_c2 = NULL;
    vx_df_image y8_r8_c2_fmt;
    vx_uint32 y8_r8_c2_w, y8_r8_c2_h;

    vx_image uv8_g8_c3 = NULL;
    vx_df_image uv8_g8_c3_fmt;
    vx_uint32 uv8_g8_c3_w, uv8_g8_c3_h;

    vx_image s8_b8_c4 = NULL;
    vx_df_image s8_b8_c4_fmt;
    vx_uint32 s8_b8_c4_w, s8_b8_c4_h;

    vx_distribution histogram = NULL;
    vx_int32 histogram_offset = 0;
    vx_uint32 histogram_range = 0;
    vx_size histogram_numBins = 0;

    vx_array h3a_aew_af = NULL;
    vx_enum h3a_aew_af_item_type;
    vx_size h3a_aew_af_capacity, h3a_aew_af_item_size;

    if ( (num != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_RAW0_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (const vx_array)parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result = (const vx_array)parameters[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw0 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_RAW0_IDX];
        raw1 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_RAW1_IDX];
        raw2 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_RAW2_IDX];
        y12 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_Y12_IDX];
        uv12_c1 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_UV12_C1_IDX];
        y8_r8_c2 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_Y8_R8_C2_IDX];
        uv8_g8_c3 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_UV8_G8_C3_IDX];
        s8_b8_c4 = (const vx_image)parameters[TIVX_KERNEL_VPAC_VISS_S8_B8_C4_IDX];
        histogram = (const vx_distribution)parameters[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af = (const vx_array)parameters[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMTYPE, &configuration_item_type, sizeof(configuration_item_type)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_CAPACITY, &configuration_capacity, sizeof(configuration_capacity)));
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMSIZE, &configuration_item_size, sizeof(configuration_item_size)));

        tivxCheckStatus(&status, vxQueryArray(ae_awb_result, VX_ARRAY_ITEMTYPE, &ae_awb_result_item_type, sizeof(ae_awb_result_item_type)));
        tivxCheckStatus(&status, vxQueryArray(ae_awb_result, VX_ARRAY_CAPACITY, &ae_awb_result_capacity, sizeof(ae_awb_result_capacity)));
        tivxCheckStatus(&status, vxQueryArray(ae_awb_result, VX_ARRAY_ITEMSIZE, &ae_awb_result_item_size, sizeof(ae_awb_result_item_size)));

        tivxCheckStatus(&status, vxQueryImage(raw0, VX_IMAGE_FORMAT, &raw0_fmt, sizeof(raw0_fmt)));
        tivxCheckStatus(&status, vxQueryImage(raw0, VX_IMAGE_WIDTH, &raw0_w, sizeof(raw0_w)));
        tivxCheckStatus(&status, vxQueryImage(raw0, VX_IMAGE_HEIGHT, &raw0_h, sizeof(raw0_h)));

        if (NULL != raw1)
        {
            tivxCheckStatus(&status, vxQueryImage(raw1, VX_IMAGE_FORMAT, &raw1_fmt, sizeof(raw1_fmt)));
            tivxCheckStatus(&status, vxQueryImage(raw1, VX_IMAGE_WIDTH, &raw1_w, sizeof(raw1_w)));
            tivxCheckStatus(&status, vxQueryImage(raw1, VX_IMAGE_HEIGHT, &raw1_h, sizeof(raw1_h)));
        }

        if (NULL != raw2)
        {
            tivxCheckStatus(&status, vxQueryImage(raw2, VX_IMAGE_FORMAT, &raw2_fmt, sizeof(raw2_fmt)));
            tivxCheckStatus(&status, vxQueryImage(raw2, VX_IMAGE_WIDTH, &raw2_w, sizeof(raw2_w)));
            tivxCheckStatus(&status, vxQueryImage(raw2, VX_IMAGE_HEIGHT, &raw2_h, sizeof(raw2_h)));
        }

        if (NULL != y12)
        {
            tivxCheckStatus(&status, vxQueryImage(y12, VX_IMAGE_FORMAT, &y12_fmt, sizeof(y12_fmt)));
            tivxCheckStatus(&status, vxQueryImage(y12, VX_IMAGE_WIDTH, &y12_w, sizeof(y12_w)));
            tivxCheckStatus(&status, vxQueryImage(y12, VX_IMAGE_HEIGHT, &y12_h, sizeof(y12_h)));
        }

        if (NULL != uv12_c1)
        {
            tivxCheckStatus(&status, vxQueryImage(uv12_c1, VX_IMAGE_FORMAT, &uv12_c1_fmt, sizeof(uv12_c1_fmt)));
            tivxCheckStatus(&status, vxQueryImage(uv12_c1, VX_IMAGE_WIDTH, &uv12_c1_w, sizeof(uv12_c1_w)));
            tivxCheckStatus(&status, vxQueryImage(uv12_c1, VX_IMAGE_HEIGHT, &uv12_c1_h, sizeof(uv12_c1_h)));
        }

        if (NULL != y8_r8_c2)
        {
            tivxCheckStatus(&status, vxQueryImage(y8_r8_c2, VX_IMAGE_FORMAT, &y8_r8_c2_fmt, sizeof(y8_r8_c2_fmt)));
            tivxCheckStatus(&status, vxQueryImage(y8_r8_c2, VX_IMAGE_WIDTH, &y8_r8_c2_w, sizeof(y8_r8_c2_w)));
            tivxCheckStatus(&status, vxQueryImage(y8_r8_c2, VX_IMAGE_HEIGHT, &y8_r8_c2_h, sizeof(y8_r8_c2_h)));
        }

        if (NULL != uv8_g8_c3)
        {
            tivxCheckStatus(&status, vxQueryImage(uv8_g8_c3, VX_IMAGE_FORMAT, &uv8_g8_c3_fmt, sizeof(uv8_g8_c3_fmt)));
            tivxCheckStatus(&status, vxQueryImage(uv8_g8_c3, VX_IMAGE_WIDTH, &uv8_g8_c3_w, sizeof(uv8_g8_c3_w)));
            tivxCheckStatus(&status, vxQueryImage(uv8_g8_c3, VX_IMAGE_HEIGHT, &uv8_g8_c3_h, sizeof(uv8_g8_c3_h)));
        }

        if (NULL != s8_b8_c4)
        {
            tivxCheckStatus(&status, vxQueryImage(s8_b8_c4, VX_IMAGE_FORMAT, &s8_b8_c4_fmt, sizeof(s8_b8_c4_fmt)));
            tivxCheckStatus(&status, vxQueryImage(s8_b8_c4, VX_IMAGE_WIDTH, &s8_b8_c4_w, sizeof(s8_b8_c4_w)));
            tivxCheckStatus(&status, vxQueryImage(s8_b8_c4, VX_IMAGE_HEIGHT, &s8_b8_c4_h, sizeof(s8_b8_c4_h)));
        }

        if (NULL != histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_BINS, &histogram_numBins, sizeof(histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_RANGE, &histogram_range, sizeof(histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_OFFSET, &histogram_offset, sizeof(histogram_offset)));
        }

        if (NULL != h3a_aew_af)
        {
            tivxCheckStatus(&status, vxQueryArray(h3a_aew_af, VX_ARRAY_ITEMTYPE, &h3a_aew_af_item_type, sizeof(h3a_aew_af_item_type)));
            tivxCheckStatus(&status, vxQueryArray(h3a_aew_af, VX_ARRAY_CAPACITY, &h3a_aew_af_capacity, sizeof(h3a_aew_af_capacity)));
            tivxCheckStatus(&status, vxQueryArray(h3a_aew_af, VX_ARRAY_ITEMSIZE, &h3a_aew_af_item_size, sizeof(h3a_aew_af_item_size)));
        }
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ( configuration_item_size != sizeof(tivx_vpac_viss_params_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an array of type:\n tivx_vpac_viss_params_t \n");
        }

        if ( ae_awb_result_item_size != sizeof(tivx_ae_awb_params_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'ae_awb_result' should be an array of type:\n tivx_ae_awb_params_t \n");
        }

        if( (VX_DF_IMAGE_U8 != raw0_fmt) &&
            (VX_DF_IMAGE_U16 != raw0_fmt) &&
            (TIVX_DF_IMAGE_P12 != raw0_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'raw0' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
        }

        if (NULL != raw1)
        {
            if( (VX_DF_IMAGE_U8 != raw1_fmt) &&
                (VX_DF_IMAGE_U16 != raw1_fmt) &&
                (TIVX_DF_IMAGE_P12 != raw1_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'raw1' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != raw2)
        {
            if( (VX_DF_IMAGE_U8 != raw2_fmt) &&
                (VX_DF_IMAGE_U16 != raw2_fmt) &&
                (TIVX_DF_IMAGE_P12 != raw2_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'raw2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != y12)
        {
            if( (VX_DF_IMAGE_U16 != y12_fmt) &&
                (TIVX_DF_IMAGE_P12 != y12_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'y12' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != uv12_c1)
        {
            if( (VX_DF_IMAGE_U16 != uv12_c1_fmt) &&
                (TIVX_DF_IMAGE_P12 != uv12_c1_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'uv12_c1' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != y8_r8_c2)
        {
            if( (VX_DF_IMAGE_U8 != y8_r8_c2_fmt) &&
                (VX_DF_IMAGE_U16 != y8_r8_c2_fmt) &&
                (TIVX_DF_IMAGE_P12 != y8_r8_c2_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'y8_r8_c2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != uv8_g8_c3)
        {
            if( (VX_DF_IMAGE_U8 != uv8_g8_c3_fmt) &&
                (VX_DF_IMAGE_U16 != uv8_g8_c3_fmt) &&
                (TIVX_DF_IMAGE_P12 != uv8_g8_c3_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'uv8_g8_c3' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != s8_b8_c4)
        {
            if( (VX_DF_IMAGE_U8 != s8_b8_c4_fmt) &&
                (VX_DF_IMAGE_U16 != s8_b8_c4_fmt) &&
                (TIVX_DF_IMAGE_P12 != s8_b8_c4_fmt))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'s8_b8_c4' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }


        if (NULL != h3a_aew_af)
        {
            if ( h3a_aew_af_item_size < sizeof(tivx_h3a_data_t))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'h3a_aew_af' should be an array of size large enough for to contain the full H3A output:\n tivx_h3a_data_t \n");
            }
        }
    }


    /* PARAMETER RELATIONSHIP CHECKING */

    if (VX_SUCCESS == status)
    {
        if (NULL != raw1)
        {
            if (raw1_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'raw1' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (raw1_h != raw0_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'raw1' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != raw2)
        {
            if (raw2_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'raw2' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (raw2_h != raw0_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'raw2' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != y12)
        {
            if (y12_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'y12' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (y12_h != raw0_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'y12' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != y8_r8_c2)
        {
            if (y8_r8_c2_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'y8_r8_c2' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (y8_r8_c2_h != raw0_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'y8_r8_c2' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != s8_b8_c4)
        {
            if (s8_b8_c4_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 's8_b8_c4' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (s8_b8_c4_h != raw0_h)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 's8_b8_c4' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != uv12_c1)
        {
            if (uv12_c1_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'uv12_c1' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
        }

        if (NULL != uv8_g8_c3)
        {
            if (uv8_g8_c3_w != raw0_w)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'uv8_g8_c3' and 'raw0' should have the same value for VX_IMAGE_WIDTH\n");
            }
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    {
        tivx_vpac_viss_params_t params;
        vxCopyArrayRange(configuration, 0, 1, sizeof(tivx_vpac_viss_params_t), &params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        if ((NULL == raw1) && (NULL != raw2))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "If 'raw1' is NULL, 'raw2' must also be NULL\n");
        }

        if (NULL != uv12_c1)
        {
            if((0 == params.mux_uv12_c1_out) && (0 == params.chroma_out_mode))
            {
                if ((uv12_c1_h*2) != raw0_h)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameter 'uv12_c1' should have half the height of 'raw0'\n");
                }
            }
            else
            {
                if ((uv12_c1_h) != raw0_h)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'uv12_c1' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
                }
            }
        }
        if (NULL != uv8_g8_c3)
        {
            if((0 == params.mux_uv8_g8_c3_out) && (0 == params.chroma_out_mode))
            {
                if ((uv8_g8_c3_h*2) != raw0_h)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameter 'uv8_g8_c3' should have half the height of 'raw0'\n");
                }
            }
            else
            {
                if ((uv8_g8_c3_h) != raw0_h)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'uv8_g8_c3' and 'raw0' should have the same value for VX_IMAGE_HEIGHT\n");
                }
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacVissInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_RAW0_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    return status;
}

vx_status tivxAddKernelVpacViss(vx_context context)
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
                    TIVX_KERNEL_VPAC_VISS_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_VISS_MAX_PARAMS,
                    tivxAddKernelVpacVissValidate,
                    tivxAddKernelVpacVissInitialize,
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
                        VX_TYPE_DISTRIBUTION,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_VISS1);
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
    vx_vpac_viss_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacViss(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_viss_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_viss_kernel = NULL;

    return status;
}
