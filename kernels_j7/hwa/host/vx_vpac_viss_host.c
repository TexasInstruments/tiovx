/*
 *
 * Copyright (c) 2018-2021 Texas Instruments Incorporated
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
#ifdef BUILD_VPAC_VISS

#include "TI/tivx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_viss.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

static vx_kernel vx_vpac_viss_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelVpacVissValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelVpacVissInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
static vx_status VX_CALLBACK tivxAddKernelVpacVissDeinitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static void tivx_vpac_viss1_params_init(tivx_vpac_viss_params_t *prms);

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
    vx_df_image output0_fmt = (vx_df_image)VX_DF_IMAGE_U8;
    vx_uint32 output0_w, output0_h;

    vx_image output1 = NULL;
    vx_df_image output1_fmt;
    vx_uint32 output1_w, output1_h = 0;

    vx_image output2 = NULL;
    vx_df_image output2_fmt = (vx_df_image)VX_DF_IMAGE_U8;
    vx_uint32 output2_w, output2_h;

    vx_image output3 = NULL;
    vx_df_image output3_fmt;
    vx_uint32 output3_w, output3_h = 0;

    vx_image output4 = NULL;
    vx_df_image output4_fmt;
    vx_uint32 output4_w, output4_h;

    vx_distribution histogram0 = NULL;
    vx_int32 histogram0_offset = 0;
    vx_uint32 histogram0_range = 0;
    vx_size histogram0_numBins = 0;

    vx_distribution histogram1 = NULL;
    vx_int32 histogram1_offset = 0;
    vx_uint32 histogram1_range = 0;
    vx_size histogram1_numBins = 0;

    vx_distribution raw_histogram = NULL;
    vx_int32 raw_histogram_offset = 0;
    vx_uint32 raw_histogram_range = 0;
    vx_size raw_histogram_numBins = 0;

    vx_user_data_object h3a_aew_af = NULL;
    vx_char h3a_aew_af_name[VX_MAX_REFERENCE_NAME];
    vx_size h3a_aew_af_size;

    vx_user_data_object dcc_param = NULL;
    vx_char dcc_param_name[VX_MAX_REFERENCE_NAME];
    vx_size dcc_param_size;

    uint32_t fcp, outport;
    vx_image output_ptrs[5];
    int32_t i;

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
        histogram0 = (vx_distribution)parameters[TIVX_KERNEL_VPAC_VISS_HISTOGRAM0_IDX];
        histogram1 = (vx_distribution)parameters[TIVX_KERNEL_VPAC_VISS_HISTOGRAM1_IDX];
        raw_histogram = (vx_distribution)parameters[TIVX_KERNEL_VPAC_VISS_RAW_HISTOGRAM_IDX];
        h3a_aew_af = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
        dcc_param = (vx_user_data_object)parameters[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];
    }

    output_ptrs[0] = output0;
    output_ptrs[1] = output1;
    output_ptrs[2] = output2;
    output_ptrs[3] = output3;
    output_ptrs[4] = output4;

    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        if (NULL != ae_awb_result)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, (vx_enum)VX_USER_DATA_OBJECT_NAME, &ae_awb_result_name, sizeof(ae_awb_result_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &ae_awb_result_size, sizeof(ae_awb_result_size)));
        }

        tivxCheckStatus(&status, tivxQueryRawImage(raw, (vx_enum)TIVX_RAW_IMAGE_WIDTH, &raw_w, sizeof(raw_w)));
        tivxCheckStatus(&status, tivxQueryRawImage(raw, (vx_enum)TIVX_RAW_IMAGE_HEIGHT, &raw_h, sizeof(raw_h)));

        if (NULL != output0)
        {
            tivxCheckStatus(&status, vxQueryImage(output0, (vx_enum)VX_IMAGE_FORMAT, &output0_fmt, sizeof(output0_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output0, (vx_enum)VX_IMAGE_WIDTH, &output0_w, sizeof(output0_w)));
            tivxCheckStatus(&status, vxQueryImage(output0, (vx_enum)VX_IMAGE_HEIGHT, &output0_h, sizeof(output0_h)));
        }

        if (NULL != output1)
        {
            tivxCheckStatus(&status, vxQueryImage(output1, (vx_enum)VX_IMAGE_FORMAT, &output1_fmt, sizeof(output1_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output1, (vx_enum)VX_IMAGE_WIDTH, &output1_w, sizeof(output1_w)));
            tivxCheckStatus(&status, vxQueryImage(output1, (vx_enum)VX_IMAGE_HEIGHT, &output1_h, sizeof(output1_h)));
        }

        if (NULL != output2)
        {
            tivxCheckStatus(&status, vxQueryImage(output2, (vx_enum)VX_IMAGE_FORMAT, &output2_fmt, sizeof(output2_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output2, (vx_enum)VX_IMAGE_WIDTH, &output2_w, sizeof(output2_w)));
            tivxCheckStatus(&status, vxQueryImage(output2, (vx_enum)VX_IMAGE_HEIGHT, &output2_h, sizeof(output2_h)));
        }

        if (NULL != output3)
        {
            tivxCheckStatus(&status, vxQueryImage(output3, (vx_enum)VX_IMAGE_FORMAT, &output3_fmt, sizeof(output3_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output3, (vx_enum)VX_IMAGE_WIDTH, &output3_w, sizeof(output3_w)));
            tivxCheckStatus(&status, vxQueryImage(output3, (vx_enum)VX_IMAGE_HEIGHT, &output3_h, sizeof(output3_h)));
        }

        if (NULL != output4)
        {
            tivxCheckStatus(&status, vxQueryImage(output4, (vx_enum)VX_IMAGE_FORMAT, &output4_fmt, sizeof(output4_fmt)));
            tivxCheckStatus(&status, vxQueryImage(output4, (vx_enum)VX_IMAGE_WIDTH, &output4_w, sizeof(output4_w)));
            tivxCheckStatus(&status, vxQueryImage(output4, (vx_enum)VX_IMAGE_HEIGHT, &output4_h, sizeof(output4_h)));
        }

        if (NULL != histogram0)
        {
            tivxCheckStatus(&status, vxQueryDistribution(histogram0, (vx_enum)VX_DISTRIBUTION_BINS, &histogram0_numBins, sizeof(histogram0_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram0, (vx_enum)VX_DISTRIBUTION_RANGE, &histogram0_range, sizeof(histogram0_range)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram0, (vx_enum)VX_DISTRIBUTION_OFFSET, &histogram0_offset, sizeof(histogram0_offset)));
        }

        if (NULL != histogram1)
        {
            tivxCheckStatus(&status, vxQueryDistribution(histogram1, (vx_enum)VX_DISTRIBUTION_BINS, &histogram1_numBins, sizeof(histogram1_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram1, (vx_enum)VX_DISTRIBUTION_RANGE, &histogram1_range, sizeof(histogram1_range)));
            tivxCheckStatus(&status, vxQueryDistribution(histogram1, (vx_enum)VX_DISTRIBUTION_OFFSET, &histogram1_offset, sizeof(histogram1_offset)));
        }

        if (NULL != raw_histogram)
        {
            tivxCheckStatus(&status, vxQueryDistribution(raw_histogram, (vx_enum)VX_DISTRIBUTION_BINS, &raw_histogram_numBins, sizeof(raw_histogram_numBins)));
            tivxCheckStatus(&status, vxQueryDistribution(raw_histogram, (vx_enum)VX_DISTRIBUTION_RANGE, &raw_histogram_range, sizeof(raw_histogram_range)));
            tivxCheckStatus(&status, vxQueryDistribution(raw_histogram, (vx_enum)VX_DISTRIBUTION_OFFSET, &raw_histogram_offset, sizeof(raw_histogram_offset)));
        }

        if (NULL != h3a_aew_af)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, (vx_enum)VX_USER_DATA_OBJECT_NAME, &h3a_aew_af_name, sizeof(h3a_aew_af_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &h3a_aew_af_size, sizeof(h3a_aew_af_size)));
        }

        if (NULL != dcc_param)
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, (vx_enum)VX_USER_DATA_OBJECT_NAME, &dcc_param_name, sizeof(dcc_param_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &dcc_param_size, sizeof(dcc_param_size)));
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
            vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
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
                vxCopyUserDataObject(ae_awb_result, 0, sizeof(tivx_ae_awb_params_t), &ae_awb_params, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
            }
        }

        if (NULL != output0)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U16 != output0_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != output0_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 != output0_fmt)
#if defined(VPAC3) || defined (VPAC3L)
             && ((vx_df_image)VX_DF_IMAGE_U8 != output0_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_NV12 != output0_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_YUYV != output0_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != output0_fmt)
#endif
              )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3) || defined (VPAC3L)
                VX_PRINT(VX_ZONE_ERROR, "'output0' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_U8 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY\n");
#else
                VX_PRINT(VX_ZONE_ERROR, "'output0' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or TIVX_DF_IMAGE_NV12_P12\n");
#endif
            }
        }

        if (NULL != output1)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U16 != output1_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != output1_fmt)
#if defined(VPAC3) || defined (VPAC3L)
             && ((vx_df_image)VX_DF_IMAGE_U8 != output0_fmt)
#endif
                )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3) || defined (VPAC3L)
                VX_PRINT(VX_ZONE_ERROR, "'output1' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_U8 \n");
#else
                VX_PRINT(VX_ZONE_ERROR, "'output1' should be an image of type:\n VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
#endif
            }
        }

        if (NULL != output2)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U8 != output2_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != output2_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != output2_fmt) &&
#if defined(VPAC3) || defined (VPAC3L)
                ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 != output2_fmt) &&
#endif
                ((vx_df_image)VX_DF_IMAGE_NV12 != output2_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_YUYV != output2_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_UYVY != output2_fmt) )
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3) || defined (VPAC3L)
                VX_PRINT(VX_ZONE_ERROR, "'output2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or TIVX_DF_IMAGE_NV12_P12 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY \n");
#else
                VX_PRINT(VX_ZONE_ERROR, "'output2' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_NV12 or VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY \n");
#endif
            }
        }

        if (NULL != output3)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U8 != output3_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != output3_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != output3_fmt))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'output3' should be an image of type:\n VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 \n");
            }
        }

        if (NULL != output4)
        {
            if( ((vx_df_image)VX_DF_IMAGE_U8 != output4_fmt) &&
                ((vx_df_image)VX_DF_IMAGE_U16 != output4_fmt) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 != output4_fmt))
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

#if !defined (VPAC3) && !defined (VPAC3L)
    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != histogram1)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'histogram1' should be NULL for this SoC \n");
        }
        if (NULL != raw_histogram)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'raw_histogram' should be NULL for this SoC \n");
        }

        /* If user didn't use init function, or accidentally programmed VPAC3 only parameters, this sets it back to VPAC1 defautls */
        tivx_vpac_viss1_params_init(&params);
        vxCopyUserDataObject(configuration, 0, sizeof(tivx_vpac_viss_params_t), &params, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
    }
#endif

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != histogram0)
        {
            if(histogram0_numBins != 256)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'histogram0' should have 256 bins \n");
            }
        }
        if (NULL != histogram1)
        {
            if(histogram1_numBins != 256)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'histogram1' should have 256 bins \n");
            }
        }
        if (NULL != raw_histogram)
        {
            if(raw_histogram_numBins != 128)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'raw_histogram' should have 128 bins \n");
            }
        }
    }

    /* MUX VALUE CHECKING */
    if ((vx_status)VX_SUCCESS == status)
    {
        int32_t fcp_mux_status[TIVX_VPAC_VISS_FCP_NUM_INSTANCES][5] = {0};
        int32_t chroma_mux_status[TIVX_VPAC_VISS_FCP_NUM_INSTANCES] = {0};
        int32_t loop_chroma_mux_status = 0;
        vx_df_image output_fmt[3];

        for(i=0; i < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; i++)
        {
            if ((TIVX_VPAC_VISS_MUX0_Y12      != params.fcp[i].mux_output0) &&
                (TIVX_VPAC_VISS_MUX0_VALUE12  != params.fcp[i].mux_output0) &&
                (TIVX_VPAC_VISS_MUX0_NV12_P12 != params.fcp[i].mux_output0)
#if defined (VPAC3L)
             && (TIVX_VPAC_VISS_MUX0_IR8 != params.fcp[i].mux_output0)
             && (TIVX_VPAC_VISS_MUX0_IR12_P12 != params.fcp[i].mux_output0)
#endif
            )
            {
                fcp_mux_status[i][0] = 1;
            }
            if ((TIVX_VPAC_VISS_MUX1_UV12 != params.fcp[i].mux_output1) &&
                (TIVX_VPAC_VISS_MUX1_C1   != params.fcp[i].mux_output1))
            {
                fcp_mux_status[i][1] = 1;
            }
#if defined (VPAC3L)
            if (7u < params.fcp[i].mux_output2)
#else
            if (5u < params.fcp[i].mux_output2)
#endif
            {
                fcp_mux_status[i][2] = 1;
            }
            if (2u < params.fcp[i].mux_output3)
            {
                fcp_mux_status[i][3] = 1;
            }
            if ((TIVX_VPAC_VISS_MUX4_BLUE != params.fcp[i].mux_output4) &&
                (TIVX_VPAC_VISS_MUX4_C4   != params.fcp[i].mux_output4) &&
                (TIVX_VPAC_VISS_MUX4_SAT  != params.fcp[i].mux_output4))
            {
                fcp_mux_status[i][4] = 1;
            }
        }

        for (i=0; i<5; i++)
        {
            if (NULL != output_ptrs[i])
            {
                fcp     = params.output_fcp_mapping[i] & 1U;
                outport = params.output_fcp_mapping[i] & 2U;

                if((i & 1) == 1) /* if i is 1 or 3 */
                {
                    outport++;
                }
                else if (i == 4)
                {
                    outport = i;
                }

                if ( (fcp >= TIVX_VPAC_VISS_FCP_NUM_INSTANCES) ||
                     (0 != fcp_mux_status[fcp][outport]))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3)
                    VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for output_fcp_mapping[%d] OR \n", i);
#endif
                    VX_PRINT(VX_ZONE_ERROR, "Invalid mux value for fcp[%d].mux_output%d\n", fcp, outport);
                }
            }
        }

        output_fmt[0] = output0_fmt;
        output_fmt[2] = output2_fmt;

        for (i=0; i<3; i+=2) /* Iterate twice, once for output0, once for output2 */
        {
            if (NULL != output_ptrs[i])
            {
                fcp     = params.output_fcp_mapping[i] & 1U;
                outport = params.output_fcp_mapping[i] & 2U;

                if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == output_fmt[i]) &&
                    ((0 != outport) || (TIVX_VPAC_VISS_MUX0_NV12_P12 != params.fcp[fcp].mux_output0)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3)
                    VX_PRINT(VX_ZONE_ERROR, "params.output_fcp_mapping[%d] must be set to output0 for NV12_P12 output of output0\n", i);
#endif
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output0 must be set to TIVX_VPAC_VISS_MUX0_NV12_P12 for NV12_P12 output of output%d\n", fcp, i);
                }
#if defined(VPAC3L)
                if (((vx_df_image)VX_DF_IMAGE_U8 == output_fmt[i]) &&
                    ((0 != outport) || (TIVX_VPAC_VISS_MUX0_IR8 != params.fcp[fcp].mux_output0)) &&
                    (params.enable_ir_op == TIVX_VPAC_VISS_IR_ENABLE))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output0 must be set to TIVX_VPAC_VISS_MUX0_IR8 for 8 bit IR output of output%d\n", fcp, i);
                }
                if (((vx_df_image)TIVX_DF_IMAGE_P12 == output_fmt[i]) &&
                    ((0 != outport) || (TIVX_VPAC_VISS_MUX0_IR12_P12 != params.fcp[fcp].mux_output0)) &&
                    (params.enable_ir_op == TIVX_VPAC_VISS_IR_ENABLE))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output0 must be set to TIVX_VPAC_VISS_MUX0_IR12_P12 for Packed 12 bit IR output of output%d\n", fcp, i);
                }
                if (((vx_df_image)VX_DF_IMAGE_U16 == output_fmt[i]) &&
                    ((2 != outport) || (TIVX_VPAC_VISS_MUX2_IR12_U16 != params.fcp[fcp].mux_output2)) &&
                    (params.enable_ir_op == TIVX_VPAC_VISS_IR_ENABLE))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output2 must be set to TIVX_VPAC_VISS_MUX2_IR12_U16 for 12 bit IR output in 16 bit container of output%d\n", fcp, i);
                }
#endif
                if (((vx_df_image)VX_DF_IMAGE_NV12 == output_fmt[i]) &&
                    ((2 != outport) || (TIVX_VPAC_VISS_MUX2_NV12 != params.fcp[fcp].mux_output2)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3)
                    VX_PRINT(VX_ZONE_ERROR, "params.output_fcp_mapping[%d] must be set to output2 for NV12 output of output0\n", i);
#endif
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output2 must be set to TIVX_VPAC_VISS_MUX2_NV12 for NV12 output of output%d\n", fcp, i);
                }
                if ((((vx_df_image)VX_DF_IMAGE_YUYV == output_fmt[i]) || ((vx_df_image)VX_DF_IMAGE_UYVY == output_fmt[i])) &&
                    ((2 != outport) || (TIVX_VPAC_VISS_MUX2_YUV422 != params.fcp[fcp].mux_output2)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
#if defined(VPAC3)
                    VX_PRINT(VX_ZONE_ERROR, "params.output_fcp_mapping[%d] must be set to output2 for YUYV or UYVY output of output0\n", i);
#endif
                    VX_PRINT(VX_ZONE_ERROR, "fcp[%d].mux_output2 must be set to TIVX_VPAC_VISS_MUX2_YUV422 for YUYV or UYVY output of output%d\n", fcp, i);
                }
            }
        }

        for(i=0; i < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; i++)
        {
            if ((TIVX_VPAC_VISS_MUX0_NV12_P12 == params.fcp[i].mux_output0) &&
                (TIVX_VPAC_VISS_MUX2_YUV422 == params.fcp[i].mux_output2))
            {
                chroma_mux_status[i] = 1;
                loop_chroma_mux_status = 1;
            }
        }

        if (0 != loop_chroma_mux_status)
        {
            uint32_t fcp_of_0, fcp_of_2;
            fcp_of_0 = params.output_fcp_mapping[0] & 1U;
            fcp_of_2 = params.output_fcp_mapping[2] & 1U;

            if ((fcp_of_0 == fcp_of_2) && (0 != chroma_mux_status[fcp_of_0]))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "NV12_P12 on 'output0' and YUV422 on 'output2' are not possible\n");
            }
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

            if ((((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == output0_fmt) || ((vx_df_image)VX_DF_IMAGE_NV12 == output0_fmt)) &&
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

            if ((((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == output2_fmt) || ((vx_df_image)VX_DF_IMAGE_NV12 == output2_fmt)) &&
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
        uint32_t expected_height[TIVX_VPAC_VISS_FCP_NUM_INSTANCES][4];
        uint32_t output_height[4];

        output_height[1] = output1_h;
        output_height[3] = output3_h;

        for(i=0; i < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; i++)
        {
            uint32_t j;

            for (j=0;j<4;j++)
            {
                expected_height[i][j] = raw_h;
            }

            if((TIVX_VPAC_VISS_MUX1_UV12 == params.fcp[i].mux_output1) &&
               (TIVX_VPAC_VISS_CHROMA_MODE_420 == params.fcp[i].chroma_mode))
            {
                expected_height[i][1] = raw_h/2;
            }

            if((TIVX_VPAC_VISS_MUX3_UV8 == params.fcp[i].mux_output3) &&
               (TIVX_VPAC_VISS_CHROMA_MODE_420 == params.fcp[i].chroma_mode))
            {
                expected_height[i][3] = raw_h/2;
            }
        }

        for (i=1; i<4; i+=2) /* Iterate twice, once for output1, once for output3 */
        {
            if (NULL != output_ptrs[i])
            {
                fcp     = params.output_fcp_mapping[i] & 1U;
                outport = (params.output_fcp_mapping[i] & 2U) + 1U;

                if (output_height[i] != expected_height[fcp][outport])
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Parameter 'output%d' height should have value of %d\n", i, expected_height[fcp][i]);
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        int32_t i;

        for(i=0; i<4; i++)
        {
            if (3U < params.output_fcp_mapping[i])
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter output_fcp_mapping[%d] should be in range [0-3]\n", i);
            }
        }
        if (1U < params.output_fcp_mapping[4])
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter output_fcp_mapping[4] should be either 0 or 1\n");
        }
        if (4U < params.fcp1_config)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter fcp1_config should be in range [0-4]\n");
        }
#if defined (VPAC3L)
        if (1U < params.enable_ir_op)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter enable_ir_op should be either 0 or 1\n");
        }
        if (1U < params.enable_bayer_op)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter enable_bayer_op should be either 0 or 1\n");
        }
#endif
        if (1U < params.bypass_cac)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter bypass_cac should be either 0 or 1\n");
        }
        if (1U < params.bypass_dwb)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter bypass_dwb should be either 0 or 1\n");
        }
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
#if defined (VPAC3L)
        if (1U < params.bypass_pcid)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter bypass_pcid should be either 0 or 1\n");
        }
#endif
        if ((NULL != h3a_aew_af) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW0 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW1 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_RAW2 != params.h3a_in) &&
            (TIVX_VPAC_VISS_H3A_IN_LSC != params.h3a_in)
#if defined (VPAC3L)
         && (TIVX_VPAC_VISS_H3A_IN_PCID != params.h3a_in)
#endif
        )
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

        for(i=0; i<TIVX_VPAC_VISS_FCP_NUM_INSTANCES; i++)
        {
            if ((TIVX_VPAC_VISS_EE_MODE_OFF != params.fcp[i].ee_mode) &&
                (TIVX_VPAC_VISS_EE_MODE_Y12 != params.fcp[i].ee_mode) &&
                (TIVX_VPAC_VISS_EE_MODE_Y8 != params.fcp[i].ee_mode))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter fcp[%d].ee_mode should be either:\n TIVX_VPAC_VISS_EE_MODE_OFF or TIVX_VPAC_VISS_EE_MODE_Y12 or TIVX_VPAC_VISS_EE_MODE_Y8\n", i);
            }
            if (((TIVX_VPAC_VISS_MUX1_UV12 == params.fcp[i].mux_output1) || (TIVX_VPAC_VISS_MUX3_UV8 == params.fcp[i].mux_output3)) &&
                (TIVX_VPAC_VISS_CHROMA_MODE_420 != params.fcp[i].chroma_mode) &&
                (TIVX_VPAC_VISS_CHROMA_MODE_422 != params.fcp[i].chroma_mode))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Parameter fcp[%d].chroma_mode should be either:\n TIVX_VPAC_VISS_CHROMA_MODE_420 or TIVX_VPAC_VISS_CHROMA_MODE_422\n", i);
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != ae_awb_result)
        {
            if ((TIVX_VPAC_VISS_H3A_IN_RAW0 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_RAW1 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_RAW2 != ae_awb_params.h3a_source_data) &&
                (TIVX_VPAC_VISS_H3A_IN_LSC != ae_awb_params.h3a_source_data)
#if defined (VPAC3L)
            && (TIVX_VPAC_VISS_H3A_IN_PCID != ae_awb_params.h3a_source_data)
#endif
                )
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

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxLogRtTraceKernelInstanceAddEvent(node, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA, "VISS_HWA");
        tivxLogRtTraceKernelInstanceAddEvent(node, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA, "VISS_DMA");
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelVpacVissDeinitialize(vx_node node,
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

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxLogRtTraceKernelInstanceRemoveEvent(node, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA);
        tivxLogRtTraceKernelInstanceRemoveEvent(node, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_DMA);
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
                    tivxAddKernelVpacVissDeinitialize);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            /* Configuration */
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
            /* AE/AEWB Result */
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* DCC Buffer */
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* Input RAW Images */
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        TIVX_TYPE_RAW_IMAGE,
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
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* H3A Output */
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* Histogram0 Output */
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
            /* Histogram1 Output */
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
            /* Raw Histogram Output */
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_VISS1);
            #if defined(SOC_J784S4)
            tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_VISS1);
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

/* These default allow VPAC3 to function like VPAC1 (mostly, build defines are also needed) */
static void tivx_vpac_viss1_params_init(tivx_vpac_viss_params_t *prms)
{
    prms->output_fcp_mapping[0] =  0; /* TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0, TIVX_VPAC_VISS_FCP_OUT0);*/
    prms->output_fcp_mapping[1] =  0; /* TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0, TIVX_VPAC_VISS_FCP_OUT1);*/
    prms->output_fcp_mapping[2] = 2U; /* TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0, TIVX_VPAC_VISS_FCP_OUT2);*/
    prms->output_fcp_mapping[3] = 2U; /* TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0, TIVX_VPAC_VISS_FCP_OUT3);*/
    prms->output_fcp_mapping[4] =  0; /* TIVX_VPAC_VISS_MAP_FCP_OUTPUT(TIVX_VPAC_VISS_FCP0, TIVX_VPAC_VISS_FCP_OUT4);*/

    prms->bypass_cac = 1U;
    prms->bypass_dwb = 1U;

#ifdef VPAC3L
    prms->bypass_pcid = 1U;
    prms->enable_ir_op = TIVX_VPAC_VISS_IR_DISABLE;
    prms->enable_bayer_op = TIVX_VPAC_VISS_BAYER_ENABLE;
#endif

}

void tivx_vpac_viss_params_init(tivx_vpac_viss_params_t *prms)
{
    if (NULL != prms)
    {

        uint32_t i;
        memset(prms, 0x0, sizeof(tivx_vpac_viss_params_t));

        tivx_vpac_viss1_params_init(prms);

        for(i=0; i< TIVX_VPAC_VISS_FCP_NUM_INSTANCES; i++)
        {
            prms->fcp[i].mux_output0 = TIVX_VPAC_VISS_MUX0_NV12_P12;
            prms->fcp[i].mux_output2 = TIVX_VPAC_VISS_MUX2_NV12;
            prms->fcp[i].mux_output4 = TIVX_VPAC_VISS_MUX4_SAT;
            prms->fcp[i].ee_mode     = TIVX_VPAC_VISS_EE_MODE_OFF;
            prms->fcp[i].chroma_mode = TIVX_VPAC_VISS_CHROMA_MODE_420;
        }

        prms->bypass_glbce = 1U;
        prms->bypass_nsf4 = 1U;

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

#endif
