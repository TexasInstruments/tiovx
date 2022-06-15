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

#include <stdio.h>
#include "TI/tivx.h"
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_viss.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "viss_top.h"
#include "idcc.h"

#include "vx_vpac_viss_target_sim_priv.h"

#define NO_SHIFT 0
#define UP_SHIFT 1
#define DOWN_SHIFT 2

#define H3A_AEW_CONFIG_SIZE     (12) /* sizeof(tivx_h3a_aew_config) */


typedef struct
{
    viss_config config;
    vx_uint32 use_dcc;
    uint8_t * dcc_out_buf;
    vx_uint32 dcc_out_numbytes;
    dcc_parser_input_params_t * dcc_input_params;
    dcc_parser_output_params_t * dcc_output_params;

    uint32_t viss_frame_count;
} tivxVpacVissParams;

static tivx_target_kernel vx_vpac_viss_target_kernel = NULL;

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacVissFreeMem(tivxVpacVissParams *prms);

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status dcc_status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_user_data_object_t *ae_awb_result_desc;
    tivx_obj_desc_raw_image_t *raw_desc;
    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_user_data_object_t *h3a_aew_af_desc;
    tivxVpacVissParams *prms = NULL;
    tivx_vpac_viss_params_t *params;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw_desc = (tivx_obj_desc_raw_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT0_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT4_IDX];
        histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxVpacVissParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        int32_t i;
        void *configuration_target_ptr;
        void *ae_awb_result_target_ptr;
        void *raw_target_ptr[3];
        void *raw_mem_target_ptr[3];
        void *y12_target_ptr = NULL;
        void *uv12_c1_target_ptr = NULL;
        void *y8_r8_c2_target_ptr = NULL;
        void *uv8_g8_c3_target_ptr = NULL;
        void *s8_b8_c4_target_ptr = NULL;
        void *histogram_target_ptr = NULL;
        void *h3a_aew_af_target_ptr = NULL;
        uint32_t num_exposures = raw_desc->params.num_exposures;

        raw_target_ptr[0] = NULL;
        raw_target_ptr[1] = NULL;
        raw_target_ptr[2] = NULL;
        raw_mem_target_ptr[0] = NULL;
        raw_mem_target_ptr[1] = NULL;
        raw_mem_target_ptr[2] = NULL;

        configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        ae_awb_result_target_ptr = tivxMemShared2TargetPtr(&ae_awb_result_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(ae_awb_result_target_ptr,
           ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        /* Get image pointer(s) */
        for(i=0; i < num_exposures; i++)
        {
            raw_target_ptr[i] = tivxMemShared2TargetPtr(&raw_desc->img_ptr[i]);
        }

        /* Map buffer(s) */
        raw_mem_target_ptr[0] = tivxMemShared2TargetPtr(&raw_desc->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(raw_mem_target_ptr[0],
            raw_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if (!raw_desc->params.line_interleaved)
        {
            for(i=1; i < num_exposures; i++)
            {
                raw_mem_target_ptr[i] = tivxMemShared2TargetPtr(&raw_desc->mem_ptr[i]);
                tivxCheckStatus(&status, tivxMemBufferMap(raw_mem_target_ptr[i],
                    raw_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));
            }
        }

        if( y12_desc != NULL)
        {
            y12_target_ptr = tivxMemShared2TargetPtr(&y12_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(y12_target_ptr,
               y12_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format)
            {
                uv12_c1_target_ptr = tivxMemShared2TargetPtr(&y12_desc->mem_ptr[1]);
                tivxCheckStatus(&status, tivxMemBufferMap(uv12_c1_target_ptr,
                  y12_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                   (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv12_c1_desc != NULL)
        {
            uv12_c1_target_ptr = tivxMemShared2TargetPtr(&uv12_c1_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( y8_r8_c2_desc != NULL)
        {
            y8_r8_c2_target_ptr = tivxMemShared2TargetPtr(&y8_r8_c2_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if ((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format)
            {
                uv8_g8_c3_target_ptr = tivxMemShared2TargetPtr(&y8_r8_c2_desc->mem_ptr[1]);
                tivxCheckStatus(&status, tivxMemBufferMap(uv8_g8_c3_target_ptr,
                  y8_r8_c2_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                   (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv8_g8_c3_desc != NULL)
        {
            uv8_g8_c3_target_ptr = tivxMemShared2TargetPtr(&uv8_g8_c3_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( s8_b8_c4_desc != NULL)
        {
            s8_b8_c4_target_ptr = tivxMemShared2TargetPtr(&s8_b8_c4_desc->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram_desc != NULL)
        {
            histogram_target_ptr = tivxMemShared2TargetPtr(&histogram_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(histogram_target_ptr,
               histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( h3a_aew_af_desc != NULL)
        {
            h3a_aew_af_target_ptr = tivxMemShared2TargetPtr(&h3a_aew_af_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(h3a_aew_af_target_ptr,
               h3a_aew_af_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        params = (tivx_vpac_viss_params_t *)configuration_target_ptr;

        /* call kernel processing function */

        /* Read non-NUll input buffers (up to 3) */
        lse_reformat_in_viss(raw_desc, raw_target_ptr[0], prms->config.buffer[0], 0);
        if (num_exposures > 1U)
        {
            lse_reformat_in_viss(raw_desc, raw_target_ptr[1], prms->config.buffer[1*2], 1);
        }
        if (num_exposures > 2U)
        {
            lse_reformat_in_viss(raw_desc, raw_target_ptr[2], prms->config.buffer[2*2], 2);
        }

        /* PROCESSING */

        /*                                  |-------------------->
         *                                  |        |----------->
         * RAWFE -> NSF4 -> GLBCE -> FLEXCFA -> FLEXCC -> EE ---->
         *     '--> H3A --->                         '--> HIST -->
         */

        /* RAWFE */
        if(0 == params->h3a_in)
        {
            prms->config.rawfe_params.h3a_mux_sel = 2;
        }
        else if(1 == params->h3a_in)
        {
            prms->config.rawfe_params.h3a_mux_sel = 1;
        }
        else if(2 == params->h3a_in)
        {
            prms->config.rawfe_params.h3a_mux_sel = 0;
        }
        else
        {
            prms->config.rawfe_params.h3a_mux_sel = 3;
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != histogram_desc))
        {
            prms->config.hist = histogram_target_ptr;
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != h3a_aew_af_desc))
        {
            tivx_h3a_data_t *pH3a_buf = (tivx_h3a_data_t*)h3a_aew_af_target_ptr;
            pH3a_buf->aew_af_mode = params->h3a_aewb_af_mode;
            pH3a_buf->h3a_source_data = params->h3a_in;
            pH3a_buf->size = prms->config.af_buffer_size;
            if(0 == params->h3a_aewb_af_mode)
            {
                pH3a_buf->size = prms->config.aew_buffer_size + H3A_AEW_CONFIG_SIZE;
            }
            prms->config.h3a = (uint32_t*)pH3a_buf->data;
        }

        /*Apply AWB Gains and get DCC parameters for the relevant photospace*/
        {
            tivx_ae_awb_params_t * aewb_result = (tivx_ae_awb_params_t *)ae_awb_result_target_ptr;

            /* apply AWB gains in RAWFE when NSF4 is bypassed */
            if ((1u == params->bypass_nsf4) && (1u == aewb_result->awb_valid))
            {
                prms->config.rawfe_params.wb2.gain[0] = aewb_result->wb_gains[0];
                prms->config.rawfe_params.wb2.gain[1] = aewb_result->wb_gains[1];
                prms->config.rawfe_params.wb2.gain[2] = aewb_result->wb_gains[2];
                prms->config.rawfe_params.wb2.gain[3] = aewb_result->wb_gains[3];
            }
            if(1u == prms->use_dcc)
            {
                prms->dcc_input_params->analog_gain = aewb_result->analog_gain;
                prms->dcc_input_params->cameraId = params->sensor_dcc_id;
                prms->dcc_input_params->color_temparature = aewb_result->color_temperature;
                prms->dcc_input_params->exposure_time = aewb_result->exposure_time;
                prms->dcc_input_params->analog_gain = aewb_result->analog_gain;
                dcc_status |= dcc_update(prms->dcc_input_params, prms->dcc_output_params);
            }
        }

        if(1u == prms->use_dcc)
        {
            vx_uint8 count;
            for(count=0;count<4U;count++)
            {
                prms->config.rawfe_params.pwl_lut_long.offset[count] = prms->dcc_output_params->vissBLC.l_dcoffset[count];
                prms->config.rawfe_params.pwl_lut_short.offset[count] = prms->dcc_output_params->vissBLC.s_dcoffset[count];
                prms->config.rawfe_params.pwl_lut_vshort.offset[count] = prms->dcc_output_params->vissBLC.vs_dcoffset[count];
            }
            if (1 == prms->dcc_output_params->issH3aMuxLuts.enable)
            {
                int cnt = prms->viss_frame_count % prms->dcc_output_params->issH3aMuxLuts.h3a_mux_lut_num;
                tivxVpacVissParseH3aLutParams(cnt,
                    &prms->config.rawfe_params.lut_h3a, prms->dcc_output_params);
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != h3a_aew_af_desc))
        {
            if(1u == prms->use_dcc)
            {
                /* Update H3A params using DCC config */
                /* TODO: Add an update flag so that the params are updated only when a change is detected */

                tivxVpacVissParseH3aParams(&prms->config.h3a_params,
                    prms->dcc_output_params);
            }
        }

        /* NSF4 */
        if(0 == prms->config.bypass_nsf4)
        {
            tivx_ae_awb_params_t * aewb_result = (tivx_ae_awb_params_t *)ae_awb_result_target_ptr;

            if(1u == prms->use_dcc)
            {
                tivxVpacVissParseNsf4Params(&prms->config.nsf4_params,
                    prms->dcc_output_params);
                prms->config.nsf4_params.iw = raw_desc->params.width;
                prms->config.nsf4_params.ih = raw_desc->params.height;
                if (0 == aewb_result->awb_valid)
                {
                    prms->config.nsf4_params.wb_gain[0] = prms->dcc_output_params->vissNSF4Cfg.wb_gains[0];
                    prms->config.nsf4_params.wb_gain[1] = prms->dcc_output_params->vissNSF4Cfg.wb_gains[1];
                    prms->config.nsf4_params.wb_gain[2] = prms->dcc_output_params->vissNSF4Cfg.wb_gains[2];
                    prms->config.nsf4_params.wb_gain[3] = prms->dcc_output_params->vissNSF4Cfg.wb_gains[3];
                }
            }

            if (1 == aewb_result->awb_valid)
            {
                prms->config.nsf4_params.wb_gain[0] = aewb_result->wb_gains[0];
                prms->config.nsf4_params.wb_gain[1] = aewb_result->wb_gains[1];
                prms->config.nsf4_params.wb_gain[2] = aewb_result->wb_gains[2];
                prms->config.nsf4_params.wb_gain[3] = aewb_result->wb_gains[3];
            }
        }

        /* FLEXCC */
        if(0 == prms->config.bypass_cc)
        {
            if(1u == prms->use_dcc)
            {
                if(prms->dcc_output_params->useCcmCfg)
                {
                    tivxVpacVissParseCCMParams(&prms->config.flexcc_params.CCM1,
                        prms->dcc_output_params);
                }
            }
        }

        status = vlab_hwa_process(VPAC_VISS_BASE_ADDRESS, "VPAC_VISS", sizeof(viss_config), &prms->config);

        /* Fill non-NULL output buffers (up to 7) */

        if( y12_desc != NULL)
        {
            if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format)
            {
                lse_reformat_out_viss(raw_desc, y12_desc, y12_target_ptr, uv12_c1_target_ptr, prms->config.buffer[3*2], prms->config.buffer[4*2], 12);
            }
            else
            {
                lse_reformat_out_viss(raw_desc, y12_desc, y12_target_ptr, NULL, prms->config.buffer[3*2], NULL, 12);
            }
        }
        if( uv12_c1_desc != NULL)
        {
            lse_reformat_out_viss(raw_desc, uv12_c1_desc, uv12_c1_target_ptr, NULL, prms->config.buffer[4*2], NULL, 12);
        }
        if( y8_r8_c2_desc != NULL)
        {
            uint16_t  out_y8_r8_c2_bit_align = 8;
            if(2 == params->mux_output2)
            {
                out_y8_r8_c2_bit_align = 12;
            }
            if (((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format) ||
                ((vx_df_image)VX_DF_IMAGE_YUYV == y8_r8_c2_desc->format) ||
                ((vx_df_image)VX_DF_IMAGE_UYVY == y8_r8_c2_desc->format))
            {
                lse_reformat_out_viss(raw_desc, y8_r8_c2_desc, y8_r8_c2_target_ptr, uv8_g8_c3_target_ptr, prms->config.buffer[5*2], prms->config.buffer[6*2], out_y8_r8_c2_bit_align);
            }
            else
            {
                lse_reformat_out_viss(raw_desc, y8_r8_c2_desc, y8_r8_c2_target_ptr, NULL, prms->config.buffer[5*2], NULL, out_y8_r8_c2_bit_align);
            }
        }
        if( uv8_g8_c3_desc != NULL)
        {
            uint16_t  out_uv8_g8_c3_bit_align = 8;
            if(2 == params->mux_output3)
            {
                out_uv8_g8_c3_bit_align = 12;
            }
            lse_reformat_out_viss(raw_desc, uv8_g8_c3_desc, uv8_g8_c3_target_ptr, NULL, prms->config.buffer[6*2], NULL, out_uv8_g8_c3_bit_align);
        }
        if( s8_b8_c4_desc != NULL)
        {
            uint16_t  out_s8_b8_c4_bit_align = 8;
            if(2 == params->mux_output4)
            {
                out_s8_b8_c4_bit_align = 12;
            }
            lse_reformat_out_viss(raw_desc, s8_b8_c4_desc, s8_b8_c4_target_ptr, NULL, prms->config.buffer[7*2], NULL, out_s8_b8_c4_bit_align);
        }

        prms->viss_frame_count++;

        /* kernel processing function complete */

        tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(ae_awb_result_target_ptr,
           ae_awb_result_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(raw_mem_target_ptr[0],
            raw_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
            (vx_enum)VX_READ_ONLY));

        if (!raw_desc->params.line_interleaved)
        {
            for(i=1; i < num_exposures; i++)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(raw_mem_target_ptr[i],
                    raw_desc->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));
            }
        }

        if( y12_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(y12_target_ptr,
               y12_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == y12_desc->format)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(uv12_c1_target_ptr,
                   y12_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv12_c1_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( y8_r8_c2_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            if ((vx_df_image)VX_DF_IMAGE_NV12 == y8_r8_c2_desc->format)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(uv8_g8_c3_target_ptr,
                   y8_r8_c2_desc->mem_size[1], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }
        }

        if( uv8_g8_c3_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( s8_b8_c4_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( histogram_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(histogram_target_ptr,
               histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        if( h3a_aew_af_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(h3a_aew_af_target_ptr,
               h3a_aew_af_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status dcc_status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_user_data_object_t *ae_awb_result_desc;
    tivx_obj_desc_raw_image_t *raw_desc;
    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_user_data_object_t *h3a_aew_af_desc;
    tivx_obj_desc_user_data_object_t *dcc_desc;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        tivxVpacVissParams *prms = NULL;
        tivx_vpac_viss_params_t *params;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw_desc = (tivx_obj_desc_raw_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX];
        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT0_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_OUT4_IDX];
        histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];
        dcc_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_DCC_BUF_IDX];

        prms = tivxMemAlloc(sizeof(tivxVpacVissParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            uint32_t width = raw_desc->imagepatch_addr[0].dim_x;
            uint32_t height = raw_desc->imagepatch_addr[0].dim_y;
            uint32_t i;
            uint32_t num_exposures = raw_desc->params.num_exposures;

            memset(prms, 0, sizeof(tivxVpacVissParams));
            prms->dcc_input_params = (dcc_parser_input_params_t *)tivxMemAlloc(sizeof(dcc_parser_input_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            if(NULL == prms->dcc_input_params)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory Allocation Failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            prms->dcc_output_params = (dcc_parser_output_params_t *)tivxMemAlloc(sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            if(NULL == prms->dcc_output_params)
            {
                VX_PRINT(VX_ZONE_ERROR, "Memory Allocation Failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            prms->config.buffer_size = (width * height) * 2;
            prms->config.magic = 0xC0DEFACE;

            for(i=0; i<num_exposures; i++)
            {
                if ((vx_status)VX_SUCCESS == status)
                {
                    prms->config.buffer[i*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->config.buffer[i*2])
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != y12_desc))
            {
                prms->config.buffer[3*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->config.buffer[3*2])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != uv12_c1_desc))
            {
                prms->config.buffer[4*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->config.buffer[4*2])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != y8_r8_c2_desc))
            {
                prms->config.buffer[5*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->config.buffer[5*2])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != uv8_g8_c3_desc))
            {
                prms->config.buffer[6*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->config.buffer[6*2])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != s8_b8_c4_desc))
            {
                prms->config.buffer[7*2] = tivxMemAlloc(prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->config.buffer[7*2])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                void *configuration_target_ptr;
                void *ae_awb_result_target_ptr;

                configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
                ae_awb_result_target_ptr = tivxMemShared2TargetPtr(&ae_awb_result_desc->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                tivxCheckStatus(&status, tivxMemBufferMap(ae_awb_result_target_ptr, ae_awb_result_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                params = (tivx_vpac_viss_params_t *)configuration_target_ptr;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                if(NULL != dcc_desc)
                {
                    prms->use_dcc = 1u;
                    prms->dcc_input_params->analog_gain = 1000;
                    prms->dcc_input_params->cameraId = params->sensor_dcc_id;
                    prms->dcc_input_params->color_temparature = 5000;
                    prms->dcc_input_params->exposure_time = 33333;

                    prms->dcc_input_params->dcc_buf_size = dcc_desc->mem_size;
                    prms->dcc_input_params->dcc_buf = tivxMemShared2TargetPtr(&dcc_desc->mem_ptr);
                    if(NULL != prms->dcc_input_params->dcc_buf)
                    {
                        tivxCheckStatus(&status, tivxMemBufferMap(prms->dcc_input_params->dcc_buf, dcc_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

                        prms->dcc_out_numbytes = calc_dcc_outbuf_size());
                        prms->dcc_out_buf = (uint8_t *)tivxMemAlloc(prms->dcc_out_numbytes, (vx_enum)TIVX_MEM_EXTERNAL);
                        if(NULL != prms->dcc_out_buf)
                        {
                            dcc_status |= Dcc_Create(prms->dcc_output_params, prms->dcc_out_buf);
                            dcc_status |= dcc_update(prms->dcc_input_params, prms->dcc_output_params);
                        }
                        else
                        {
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                        }

                        tivxCheckStatus(&status, tivxMemBufferUnmap(prms->dcc_input_params->dcc_buf, dcc_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
                    }
                    else
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS);
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_WARNING, "No DCC buffer passed. Using defaults \n");
                }

                /*                                  |-------------------->
                 *                                  |        |----------->
                 * RAWFE -> NSF4 -> GLBCE -> FLEXCFA -> FLEXCC -> EE ---->
                 *     '--> H3A --->                         '--> HIST -->
                 */

                /* RAWFE */
                tivxVpacVissParseRfeParams(&prms->config.rawfe_params, prms->dcc_output_params);
                prms->config.rawfe_params.width = width;
                prms->config.rawfe_params.height = height;
                prms->config.rawfe_params.h3a_mux_sel = params->h3a_in;

                /* H3A */
                if( h3a_aew_af_desc != NULL)
                {
                    int32_t num_aew_windows, ae_size, af_pad;

                    tivxVpacVissParseH3aParams(&prms->config.h3a_params,
                        prms->dcc_output_params);
                    /* prms->h3a_in.image_width = width;
                    prms->h3a_in.image_height = height;
                    prms->h3a_in.image_data = (int16_t*)prms->scratch_rawfe_h3a_out; */

                    /* Compute AEW buffer size */
                    num_aew_windows = (prms->config.h3a_params.aewwin1_WINVC+1)*(prms->config.h3a_params.aewwin1_WINHC);
                    ae_size = 0;
                    for (i = 0; i < num_aew_windows; i++)
                    {
                        if ( (prms->config.h3a_params.aew_cfg_AEFMT == 0) || (prms->config.h3a_params.aew_cfg_AEFMT == 1))
                        {
                            ae_size += 8;
                        }
                        else
                        {
                            ae_size += 4;
                        }
                        if (i%8 == 7 && i > 0)
                        {
                            ae_size += 4;
                        }
                        if (i%prms->config.h3a_params.aewwin1_WINHC == (prms->config.h3a_params.aewwin1_WINHC -1) && ae_size%8 == 4 && i != (num_aew_windows-1))
                        {
                            ae_size += 4;
                        }
                    }
                    if (num_aew_windows%8 != 0)
                    {
                        ae_size += 4;
                    }
                    if (ae_size %8 != 0)
                    {
                        ae_size+= 4;
                    }
                    prms->config.aew_buffer_size = ae_size * sizeof(uint32_t);

                    /* Compute AF buffer size */
                    if (prms->config.h3a_params.pcr_AF_VF_EN)
                    {
                        prms->config.af_buffer_size = (16U * (prms->config.h3a_params.afpax2_PAXHC * prms->config.h3a_params.afpax2_PAXVC)) * sizeof(uint32_t);
                    }
                    else
                    {
                        af_pad = 0;
                        if (1U == (prms->config.h3a_params.afpax2_PAXHC%2))
                        {
                            af_pad = 4*prms->config.h3a_params.afpax2_PAXVC;
                        }
                        prms->config.af_buffer_size = ((12 * (prms->config.h3a_params.afpax2_PAXHC * prms->config.h3a_params.afpax2_PAXVC)) + af_pad) * sizeof(uint32_t);
                    }

                    if ((vx_status)VX_SUCCESS == status)
                    {
                        uint32_t full_aew_size = prms->config.aew_buffer_size + H3A_AEW_CONFIG_SIZE;
                        prms->config.h3a_buffer_size = prms->config.af_buffer_size;
                        if((full_aew_size) > prms->config.h3a_buffer_size)
                        {
                            prms->config.h3a_buffer_size = full_aew_size;
                        }

                        if(prms->config.h3a_buffer_size > TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Required H3A output buffer size (%d bytes) is greater than TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES (%d bytes)\n",
                                                     prms->config.h3a_buffer_size, TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES);
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                        }
                    }
                }

                /* NSF4 */
                if(0 == params->bypass_nsf4)
                {
                    tivxVpacVissParseNsf4Params(&prms->config.nsf4_params, prms->dcc_output_params);
                    nsf4_check_parameters(&prms->config.nsf4_params);
                    prms->config.nsf4_params.iw = width;
                    prms->config.nsf4_params.ih = height;
                }
                else
                {
                    prms->config.bypass_nsf4 = 1;
                }

                /* GLBCE */
                /* Missing for now */

                /* FLEXCFA */
                tivxVpacVissParseFlxCfaParams(&prms->config.flexcfa_params, prms->dcc_output_params);
                prms->config.flexcfa_params.imgWidth = width;
                prms->config.flexcfa_params.imgHeight = height;

                /* FLEXCC */
                if( ( NULL != y12_desc ) ||
                   (( NULL != uv12_c1_desc )   && (2 != params->mux_output1)) ||
                   (( NULL != y8_r8_c2_desc )  && (2 != params->mux_output2)) ||
                   (( NULL != uv8_g8_c3_desc ) && (2 != params->mux_output3)) ||
                   (( NULL != s8_b8_c4_desc )  && (2 != params->mux_output4)) ||
                    ( NULL != histogram_desc))
                {
                    tivxVpacVissParseFlxCCParams(&prms->config.flexcc_params, prms->dcc_output_params);
                    prms->config.flexcc_params.inWidth = width;
                    prms->config.flexcc_params.inHeight = height;

                    if( (( NULL != y12_desc )      && (4 == params->mux_output0)) ||
                        (( NULL != y8_r8_c2_desc ) && (4 == params->mux_output2)) )
                    {
                        prms->config.flexcc_params.ChromaMode = 0; /* NV12 format is chosen on at least 1 output port */
                    }
                    else if (( NULL != y8_r8_c2_desc ) && (5 == params->mux_output2))
                    {
                        prms->config.flexcc_params.ChromaMode = 1; /* YUV422 interleaved is chosen */
                    }
                    else
                    {
                        prms->config.flexcc_params.ChromaMode = params->chroma_mode;
                    }

                    if( NULL != y12_desc )
                    {
                        if (3u == params->mux_output0)  /* Value Format */
                        {
                            prms->config.flexcc_params.MuxY12Out = 2; /* HSV value */
                        }
                        else
                        {
                            prms->config.flexcc_params.MuxY12Out = 1; /* YUV Luma */
                        }
                    }

                    if( NULL != y8_r8_c2_desc )
                    {
                        if (3u == params->mux_output2)  /* Value Format */
                        {
                            prms->config.flexcc_params.MuxY8Out = 2; /* HSV value */
                        }
                        else
                        {
                            prms->config.flexcc_params.MuxY8Out = 1; /* YUV Luma */
                        }
                    }
                }
                else
                {
                    prms->config.bypass_cc = 1;
                }

                /* EE */
                if(0 != params->ee_mode)
                {
                    if((( NULL != y12_desc )       && (1 == params->ee_mode)) ||
                       (( NULL != y8_r8_c2_desc )  && (2 == params->ee_mode) &&
                        ((0u == params->mux_output2) || (4u == params->mux_output2) ||
                            (5u == params->mux_output2))))
                    {
                        tivxVpacVissParseYeeParams(&prms->config.ee_params, prms->dcc_output_params);
                        prms->config.ee_params.width = width;
                        prms->config.ee_params.height = height;
                    }
                    else
                    {
                        prms->config.bypass_ee = 1;
                    }
                }
                else
                {
                    prms->config.bypass_ee = 1;
                }
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxVpacVissParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacVissFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacVissParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacVissParams) == size))
        {
            tivxVpacVissFreeMem(prms);
        }
    }

    return status;
}

void tivxAddTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            NULL,
                            NULL);
    }
}

void tivxRemoveTargetKernelVpacViss(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = NULL;
    }
}

static void tivxVpacVissFreeMem(tivxVpacVissParams *prms)
{
    if (NULL != prms)
    {
        uint32_t i;

        for (i = 0; i < NUM_BUFFERS; i++)
        {
            if (NULL != prms->config.buffer[i*2])
            {
                tivxMemFree(prms->config.buffer[i*2], prms->config.buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                prms->config.buffer[i*2] = NULL;
            }
        }

        if(NULL != prms->dcc_out_buf)
        {
            tivxMemFree(prms->dcc_out_buf, prms->dcc_out_numbytes, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_out_buf = NULL;
        }

        if(NULL != prms->dcc_input_params)
        {
            tivxMemFree(prms->dcc_input_params, sizeof(dcc_parser_input_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_input_params = NULL;
        }

        if(NULL != prms->dcc_output_params)
        {
            tivxMemFree(prms->dcc_output_params, sizeof(dcc_parser_output_params_t), (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dcc_output_params = NULL;
        }

        tivxMemFree(prms, sizeof(tivxVpacVissParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

