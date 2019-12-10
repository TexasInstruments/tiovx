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
#include "TI/j7.h"
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
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;
    tivx_vpac_viss_params_t params;

    vx_user_data_object ae_awb_result = NULL;
    vx_char ae_awb_result_name[VX_MAX_REFERENCE_NAME];
    vx_size ae_awb_result_size;
    tivx_ae_awb_params_t ae_awb_params;

    tivx_raw_image raw = NULL;
    vx_uint32 raw_w;
    vx_uint32 raw_h;

    vx_image output0 = NULL;
    vx_df_image output0_fmt;
    vx_uint32 output0_w, output0_h;

    vx_image output1 = NULL;
    vx_df_image output1_fmt;
    vx_uint32 output1_w, output1_h;

    vx_image output2 = NULL;
    vx_df_image output2_fmt;
    vx_uint32 output2_w, output2_h;

    vx_image output3 = NULL;
    vx_df_image output3_fmt;
    vx_uint32 output3_w, output3_h;

    vx_image output4 = NULL;
    vx_df_image output4_fmt;
    vx_uint32 output4_w, output4_h;

    vx_distribution histogram = NULL;
    vx_int32 histogram_offset = 0;
    vx_uint32 histogram_range = 0;
    vx_size histogram_numBins = 0;

    vx_user_data_object h3a_aew_af = NULL;
    vx_char h3a_aew_af_name[VX_MAX_REFERENCE_NAME];
    vx_size h3a_aew_af_size;

    vx_user_data_object dcc_param = NULL;
    vx_char dcc_param_name[VX_MAX_REFERENCE_NAME];
    vx_size dcc_param_size;

    if ( (num != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw = (tivx_raw_image)parameters[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
        output0 = (vx_image)parameters[TIVX_KERNEL_VPAC_VISS_OUT0_IDX];
        output1 = (vx_image)parameters[TIVX_KERNEL_VPAC_VISS_OUT1_IDX];
        output2 = (vx_image)parameters[TIVX_KERNEL_VPAC_VISS_OUT2_IDX];
        output3 = (vx_image)parameters[TIVX_KERNEL_VPAC_VISS_OUT3_IDX];
        output4 = (vx_image)parameters[TIVX_KERNEL_VPAC_VISS_OUT4_IDX];
        histogram = (vx_distribution)parameters[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
        dcc_param = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        if (NULL != ae_awb_result)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, VX_USER_DATA_OBJECT_NAME, &ae_awb_result_name, sizeof(ae_awb_result_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, VX_USER_DATA_OBJECT_SIZE, &ae_awb_result_size, sizeof(ae_awb_result_size)));
        }

        tivxCheckStatus(&status, tivxQueryRawImage(raw, TIVX_RAW_IMAGE_WIDTH, &raw_w, sizeof(raw_w)));
        tivxCheckStatus(&status, tivxQueryRawImage(raw, TIVX_RAW_IMAGE_HEIGHT, &raw_h, sizeof(raw_h)));

        if (NULL != output0)
        {
            tivxCheckStatus(&status, vxQueryImage(output0, VX_IMAGE_FORMAT, &output0_fmt, sizeof(output0_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output0, VX_IMAGE_WIDTH, &output0_w, sizeof(output0_w)));
            tivxCheckStatus(&status, vxQueryImage(output0, VX_IMAGE_HEIGHT, &output0_h, sizeof(output0_h)));
        }

        if (NULL != output1)
        {
            tivxCheckStatus(&status, vxQueryImage(output1, VX_IMAGE_FORMAT, &output1_fmt, sizeof(output1_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output1, VX_IMAGE_WIDTH, &output1_w, sizeof(output1_w)));
            tivxCheckStatus(&status, vxQueryImage(output1, VX_IMAGE_HEIGHT, &output1_h, sizeof(output1_h)));
        }

        if (NULL != output2)
        {
            tivxCheckStatus(&status, vxQueryImage(output2, VX_IMAGE_FORMAT, &output2_fmt, sizeof(output2_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output2, VX_IMAGE_WIDTH, &output2_w, sizeof(output2_w)));
            tivxCheckStatus(&status, vxQueryImage(output2, VX_IMAGE_HEIGHT, &output2_h, sizeof(output2_h)));
        }

        if (NULL != output3)
        {
            tivxCheckStatus(&status, vxQueryImage(output3, VX_IMAGE_FORMAT, &output3_fmt, sizeof(output3_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output3, VX_IMAGE_WIDTH, &output3_w, sizeof(output3_w)));
            tivxCheckStatus(&status, vxQueryImage(output3, VX_IMAGE_HEIGHT, &output3_h, sizeof(output3_h)));
        }

        if (NULL != output4)
        {
            tivxCheckStatus(&status, vxQueryImage(output4, VX_IMAGE_FORMAT, &output4_fmt, sizeof(output4_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output4, VX_IMAGE_WIDTH, &output4_w, sizeof(output4_w)));
            tivxCheckStatus(&status, vxQueryImage(output4, VX_IMAGE_HEIGHT, &output4_h, sizeof(output4_h)));
        }

        if (NULL != histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_BINS, &histogram_numBins, sizeof(histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_RANGE, &histogram_range, sizeof(histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram, VX_DISTRIBUTION_OFFSET, &histogram_offset, sizeof(histogram_offset)));
        }

        if (NULL != h3a_aew_af)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, VX_USER_DATA_OBJECT_NAME, &h3a_aew_af_name, sizeof(h3a_aew_af_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, VX_USER_DATA_OBJECT_SIZE, &h3a_aew_af_size, sizeof(h3a_aew_af_size)));
        }

        if (NULL != dcc_param)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, VX_USER_DATA_OBJECT_NAME, &dcc_param_name, sizeof(dcc_param_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, VX_USER_DATA_OBJECT_SIZE, &dcc_param_size, sizeof(dcc_param_size)));
        }
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_vpac_viss_params_t)) ||
            (strncmp(configuration_name, "tivx_vpac_viss_params_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_vpac_viss_params_t \n");
        }
        else
        {
            vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
        }

        if (NULL != ae_awb_result)
        {
            if ((ae_awb_result_size != sizeof(tivx_ae_awb_params_t)) ||
                (strncmp(ae_awb_result_name, "tivx_ae_awb_params_t", sizeof(ae_awb_result_name)) != 0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'ae_awb_result' should be a user_data_object of type:\n tivx_ae_awb_params_t \n");
            }
            else
            {
                vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
            }
        }

        if (NULL != output0)
        {
            if( (VX_DF_IMAGE_U16 != output0_fmt) &&
                (TIVX_DF_IMAGE_P12 != output0_fmt) &&
                (TIVX_DF_IMAGE_NV12_P12 != output0_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output0' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or TIVX_DF_IMAGE_NV12_P12\n");
            }
        }

        if (NULL != output1)
        {
            if( (VX_DF_IMAGE_U16 != output1_fmt) &&
                (TIVX_DF_IMAGE_P12 != output1_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output1' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != output2)
        {
            if( (VX_DF_IMAGE_U8 != output2_fmt) &&
                (VX_DF_IMAGE_U16 != output2_fmt) &&
                (TIVX_DF_IMAGE_P12 != output2_fmt) &&
                (VX_DF_IMAGE_NV12 != output2_fmt) &&
                (VX_DF_IMAGE_YUYV != output2_fmt) &&
                (VX_DF_IMAGE_UYVY != output2_fmt) )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY \n");
            }
        }

        if (NULL != output3)
        {
            if( (VX_DF_IMAGE_U8 != output3_fmt) &&
                (VX_DF_IMAGE_U16 != output3_fmt) &&
                (TIVX_DF_IMAGE_P12 != output3_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output3' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != output4)
        {
            if( (VX_DF_IMAGE_U8 != output4_fmt) &&
                (VX_DF_IMAGE_U16 != output4_fmt) &&
                (TIVX_DF_IMAGE_P12 != output4_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output4' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != h3a_aew_af)
        {
            if ((h3a_aew_af_size != sizeof(tivx_h3a_data_t)) ||
                (strncmp(h3a_aew_af_name, "tivx_h3a_data_t", sizeof(h3a_aew_af_name)) != 0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'h3a_aew_af' should be a user_data_object of type:\n tivx_h3a_data_t \n");
            }
        }

        if (NULL != dcc_param)
        {
            if ((dcc_param_size < 1U) ||
                (strncmp(dcc_param_name, "dcc_viss", sizeof(dcc_param_name)) != 0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'dcc_param' should be a user_data_object of name:\n dcc_viss \n");
            }
        }
    }

    /* MUX VALUE CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != output0)
        {
            if ((0u != params.mux_output0) && (3u != params.mux_output0) &&
                (4u != params.mux_output0))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for mux_output0\n");
            }
        }
        if (NULL != output1)
        {
            if ((0u != params.mux_output1) && (2u != params.mux_output1))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for mux_output1\n");
            }
        }
        if (NULL != output2)
        {
            if (5u < params.mux_output2)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for mux_output2\n");
            }
        }
        if (NULL != output3)
        {
            if (2u < params.mux_output3)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for mux_output3\n");
            }
        }
        if (NULL != output4)
        {
            if ((1u != params.mux_output4) && (2u != params.mux_output4) &&
                (3u != params.mux_output4))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for mux_output4\n");
            }
        }

        if ((NULL != output0) && (TIVX_DF_IMAGE_NV12_P12 == output0_fmt) &&
            (4u != params.mux_output0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Mux_output0 must be set to 4 for NV12 output\n");
        }
        if (NULL != output2)
        {
            if ((VX_DF_IMAGE_NV12 == output2_fmt) && (4u != params.mux_output2))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Mux_output2 must be set to 4 for NV12 output\n");
            }
            if (((VX_DF_IMAGE_YUYV == output2_fmt) || (VX_DF_IMAGE_UYVY == output2_fmt)) &&
                 (5u != params.mux_output2))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Mux_output2 must be set to 5 for YUYV or UYVY output\n");
            }
        }
        if ((NULL != output0) && (TIVX_DF_IMAGE_NV12_P12 == output0_fmt) &&
            ((NULL != output2) &&
                ((VX_DF_IMAGE_UYVY == output2_fmt) || (VX_DF_IMAGE_YUYV == output2_fmt))))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "NV12_P12 on 'output0' and YUV422 on 'output2' are not possible\n");
        }
    }

    /* PARAMETER RELATIONSHIP CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != output0)
        {
            if (output0_w != raw_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output0' and 'raw' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (output0_h != raw_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output0' and 'raw' should have the same value for VX_IMAGE_HEIGHT\n");
            }

            if ((TIVX_DF_IMAGE_NV12_P12 == output0_fmt) &&
                (output1 != NULL))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output1' cannot be enabled with NV12 output on 'output0'\n");
            }

        }

        if (NULL != output2)
        {
            if (output2_w != raw_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output2' and 'raw' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (output2_h != raw_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output2' and 'raw' should have the same value for VX_IMAGE_HEIGHT\n");
            }

            if ((VX_DF_IMAGE_NV12 == output2_fmt) &&
                (output3 != NULL))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output3' cannot be enabled with NV12 output on 'output2'\n");
            }
        }

        if (NULL != output4)
        {
            if (output4_w != raw_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output4' and 'raw' should have the same value for VX_IMAGE_WIDTH\n");
            }
            if (output4_h != raw_h)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output4' and 'raw' should have the same value for VX_IMAGE_HEIGHT\n");
            }
        }

        if (NULL != output1)
        {
            if (output1_w != raw_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output1' and 'raw' should have the same value for VX_IMAGE_WIDTH\n");
            }
        }

        if (NULL != output3)
        {
            if (output3_w != raw_w)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameters 'output3' and 'raw' should have the same value for VX_IMAGE_WIDTH\n");
            }
        }
    }

    /* CUSTOM PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != output1)
        {
            if((0 == params.mux_output1) && (0 == params.chroma_mode))
            {
                if ((output1_h*2) != raw_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameter 'output1' should have half the height of 'raw'\n");
                }
            }
            else
            {
                if ((output1_h) != raw_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output1' and 'raw' should have the same value for VX_IMAGE_HEIGHT\n");
                }
            }
        }
        if (NULL != output3)
        {
            if((0 == params.mux_output3) && (0 == params.chroma_mode))
            {
                if ((output3_h*2) != raw_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameter 'output3' should have half the height of 'raw'\n");
                }
            }
            else
            {
                if ((output3_h) != raw_h)
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameters 'output3' and 'raw' should have the same value for VX_IMAGE_HEIGHT\n");
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (1U < params.bypass_glbce)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter bypass_glbce should be either 0 or 1\n");
        }
        if (1U < params.bypass_nsf4)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter bypass_nsf4 should be either 0 or 1\n");
        }
        if ((NULL != h3a_aew_af) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW0 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW1 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW2 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_LSC != params.h3a_in))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter h3a_in should be either:\n TIVX_VPAC_VISS_H3A_IN_RAW0 or TIVX_VPAC_VISS_H3A_IN_RAW1 or TIVX_VPAC_VISS_H3A_IN_RAW2 or TIVX_VPAC_VISS_H3A_IN_LSC\n");
        }
        if ((NULL != h3a_aew_af) &&
            (TIVX_VPAC_VISS_H3A_MODE_AEWB != params.h3a_aewb_af_mode) &&
            (TIVX_VPAC_VISS_H3A_MODE_AF != params.h3a_aewb_af_mode))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter h3a_aewb_af_mode should be either:\n TIVX_VPAC_VISS_H3A_MODE_AEWB or TIVX_VPAC_VISS_H3A_MODE_AF\n");
        }
        if ((TIVX_VPAC_VISS_EE_MODE_OFF != params.ee_mode) &&
            (TIVX_VPAC_VISS_EE_MODE_Y12 != params.ee_mode) &&
            (TIVX_VPAC_VISS_EE_MODE_Y8 != params.ee_mode))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter ee_mode should be either:\n TIVX_VPAC_VISS_EE_MODE_OFF or TIVX_VPAC_VISS_EE_MODE_Y12 or TIVX_VPAC_VISS_EE_MODE_Y8\n");
        }
        if (((TIVX_VPAC_VISS_MUX1_UV12 == params.mux_output1) || (TIVX_VPAC_VISS_MUX3_UV8 == params.mux_output3)) &&
            (TIVX_VPAC_VISS_CHROMA_MODE_420 != params.chroma_mode) &&
            (TIVX_VPAC_VISS_CHROMA_MODE_422 != params.chroma_mode))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter chroma_mode should be either:\n TIVX_VPAC_VISS_CHROMA_MODE_420 or TIVX_VPAC_VISS_CHROMA_MODE_422\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != ae_awb_result)
        {
            if ((TIVX_VPAC_VISS_H3A_IN_RAW0 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_RAW1 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_RAW2 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_LSC != ae_awb_params.h3a_source_data))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "ae_awb parameter h3a_source_data should be either:\n TIVX_VPAC_VISS_H3A_IN_RAW0 or TIVX_VPAC_VISS_H3A_IN_RAW1 or TIVX_VPAC_VISS_H3A_IN_RAW2 or TIVX_VPAC_VISS_H3A_IN_LSC\n");
            }
            if (1U < ae_awb_params.ae_valid)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "ae_awb parameter ae_valid should be either 0 or 1\n");
            }
            if (1U < ae_awb_params.ae_converged)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "ae_awb parameter ae_converged should be either 0 or 1\n");
            }
            if (1U < ae_awb_params.awb_valid)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "ae_awb parameter awb_valid should be either 0 or 1\n");
            }
            if (1U < ae_awb_params.awb_converged)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "ae_awb parameter awb_converged should be either 0 or 1\n");
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacVissInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_VISS_RAW_IDX]))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
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
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            /* Configuration */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* AE/AEWB Result */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* DCC Buffer */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* Input RAW Images */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        TIVX_TYPE_RAW_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* H3A Output */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* Histogram Output */
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_DISTRIBUTION,
                        VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_VISS1);
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

void tivx_vpac_viss_params_init(tivx_vpac_viss_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_viss_params_t));

        prms->mux_output0 = 4U;
        prms->mux_output2 = 4U;
        prms->mux_output4 = 3U;
        prms->ee_mode = 0U;
        prms->chroma_mode = 0U;
        prms->enable_ctx = 0u;
    }
}

void tivx_h3a_data_init(tivx_h3a_data_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_h3a_data_t));
    }
}

void tivx_ae_awb_params_init(tivx_ae_awb_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_ae_awb_params_t));
    }
}

void tivx_h3a_aew_config_init(tivx_h3a_aew_config *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_h3a_aew_config));
    }
}
