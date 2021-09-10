/*
 *
 * Copyright (c) 2017-2021 Texas Instruments Incorporated
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

#ifndef VX_VPAC_VISS_TARGET_SIM_PRIV_H_
#define VX_VPAC_VISS_TARGET_SIM_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <vx_vpac_viss_target_priv.h>

#include "rawfe.h"
#include "glbce.h"
#include "nsf4.h"
#include "h3a_ovx.h"
#include "h3a_utils.h"
#include "flexcc_core.h"
#include "ee.h"
#ifdef VPAC3
#include "FLXD_demosaic_vpac3.h"
#include "cac_raw.h"
#include "RawHistogram.h"
#include "nsf4_wb.h"
#else
#include "FLXD_demosaic.h"
#endif


typedef struct
{
    /* Pointers to buffers allocated at create time */
    uint32_t *scratch_cfa_in;
    uint16_t *scratch_cfa_out[FLXD_NUM_FIR];
    uint16_t *scratch_cc_out[8];
    uint32_t *scratch_hist;
    uint16_t *scratch_ee_in;
    uint16_t *scratch_ee_out;
    uint16_t *scratch_ee_shift_out;

    /* Secondary pointers to buffers allocated in above list
     * (used for multiplexer assignments) */
    uint16_t *out_y12_16;
    uint16_t *out_uv12_c1_16;
    uint16_t *out_y8_r8_c2_16;
    uint16_t *out_uv8_g8_c3_16;
    uint16_t *out_s8_b8_c4_16;
    uint16_t *scratch_ee_shift_in;
    uint16_t *pScratch_cfa_in_16;

    uint16_t bypass_cc;
    uint16_t bypass_ee;

    uint16_t out_y8_r8_c2_bit_align;
    uint16_t out_uv8_g8_c3_bit_align;
    uint16_t out_s8_b8_c4_bit_align;

    uint16_t pre_copy;
    uint16_t post_copy;

} tivxVpacVissFcpPtrs;

typedef struct
{
    uint32_t width;
    uint32_t height;

    /* Pointers to buffers allocated at create time */
    uint16_t *raw0_16;
    uint16_t *raw1_16;
    uint16_t *raw2_16;
    uint16_t *scratch_rawfe_raw_out;
    uint16_t *scratch_rawfe_h3a_out;
    uint32_t *scratch_aew_result;
    uint32_t *scratch_af_result;
    uint16_t *scratch_cac_out;        // NEW
    uint32_t *scratch_raw_hist_out;   // NEW might be able to remove
    uint16_t *scratch_nsf4v_out;
    uint16_t *scratch_dwb_out;        // NEW
    uint16_t *scratch_glbce_out;
    tivxVpacVissFcpPtrs fcp[2];

    /* Secondary pointers to buffers allocated in above list
     * (used for multiplexer assignments) */
    uint16_t *pScratch_cac_out;
    uint16_t *pScratch_nsf4v_out;
    uint16_t *pScratch_dwb_out;
    uint16_t *pScratch_glbce_out;

    uint16_t *out_0_16;
    uint16_t *out_1_16;
    uint16_t *out_2_16;
    uint16_t *out_3_16;
    uint16_t *out_4_16;

    uint16_t out_0_align;
    uint16_t out_1_align;
    uint16_t out_2_align;
    uint16_t out_3_align;
    uint16_t out_4_align;

    uint32_t buffer_size;
    uint32_t aew_buffer_size;
    uint32_t af_buffer_size;

    uint16_t bypass_glbce;
    uint16_t bypass_nsf4;
    uint16_t bypass_cac;
    uint16_t bypass_dwb;

    cfg_rawfe rawfe_params;
#ifdef VPAC3
    Cac_Config cac_params;
    int32_t  cac_lut[4][2048];
    RawHistogram_Config raw_hist_params;
    Nsf4_DwbConfig dwb_params;
    int dwb_x[4][8];
    int dwb_y[4][8];
    int dwb_s[4][8];
#endif
    nsf4_settings nsf4_params;
    glbce_settings glbce_params;
    glbce_handle hGlbce;
    h3a_settings h3a_params;
    h3a_image h3a_in;
    tivx_h3a_aew_config aew_config;
    FLXD_Config flexcfa_params[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];
    Flexcc_Config flexcc_params[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];
    ee_Config ee_params[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];
    vx_uint32 use_dcc;
    uint8_t * dcc_out_buf;
    vx_uint32 dcc_out_numbytes;
    dcc_parser_input_params_t * dcc_input_params;
    dcc_parser_output_params_t * dcc_output_params;

    uint32_t viss_frame_count;

    tivxVpacVissObj vissObj;    /* Same context structure from driver version */

} tivxVpacVissParams;

vx_status tivxVpacVissSetConfigInSim(tivxVpacVissParams *prms);

#ifdef __cplusplus
}
#endif

#endif
