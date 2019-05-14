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

#include "rawfe.h"
#include "nsf4.h"
#include "h3a_ovx.h"
#include "h3a_utils.h"
#include "FLXD_demosaic.h"
#include "FLXD_config_reader.h"
#include "flexcc_core.h"
#include "flexcc_config_reader.h"
#include "ee.h"
#include "idcc.h"

#include "vx_vpac_viss_target_sim_priv.h"

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void tivxVpacVissParseWB2Params(cfg_wb2 *wb,
    dcc_parser_output_params_t *dcc_out_prms);
static void tivxVpacVissParseLscParams(cfg_lsc *lsc,
    dcc_parser_output_params_t *dcc_out_prms);
static void tivxVpacVissParseDpcParams(cfg_dpc *dpc,
    dcc_parser_output_params_t *dcc_out_prms);
static void tivxVpacVissParseRfeLutParams(uint32_t lut_id, cfg_lut *lut,
    dcc_parser_output_params_t *dcc_out_prms);
static void tivxVpacVissParseMergeParams(uint32_t merge_id, cfg_merge *merge,
    dcc_parser_output_params_t *dcc_out_prms);
static void tivxVpacVissParsePwlParams(uint32_t lut_id, cfg_pwl_lut *pwl,
    dcc_parser_output_params_t *dcc_out_prms);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint32_t gcfa_lut_20to16[] =
{
    #include "flexcfa_lut_20to16_0.txt"
};

int32_t gcfa_coeff[] =
{
    #include "flexcfa_cfa_0.txt"
};

uint32_t gflexcc_contrast_lut[] =
{
    #include "flexcc_contrast_lut_0.txt"
};

uint32_t gflexcc_lut_12to8[] =
{
    #include "flexcc_lut_12to8_0.txt"
};

int32_t gflexcc_yee_lut[] =
{
    #include "flexcc_yee_lut_0.txt"
};

uint32_t grawfe_pwl_long_lut[] =
{
    #include "rawfe_pwl_lut_long_0.txt"
};

uint32_t grawfe_pwl_short_lut[] =
{
    #include "rawfe_pwl_lut_short_0.txt"
};

uint32_t grawfe_pwl_vshort_lut[] =
{
    #include "rawfe_pwl_lut_vshort_0.txt"
};

uint32_t grawfe_lut_20to16[] =
{
    #include "rawfe_lut_20to16_0.txt"
};

int32_t grawfe_lsc_tbl[] =
{
    #include "rawfe_lsc_tbl_0.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxVpacVissParseRfeParams(cfg_rawfe *rfe_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (NULL != rfe_prms)
    {
        rfe_prms->width             = 1280u;
        rfe_prms->height            = 720u;
        rfe_prms->lut_shadow_en     = 0u;

        /* Parse Long exposure PWL Parameters */
        tivxVpacVissParsePwlParams(0, &rfe_prms->pwl_lut_long, dcc_out_prms);
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissParsePwlParams(1, &rfe_prms->pwl_lut_short, dcc_out_prms);
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissParsePwlParams(2, &rfe_prms->pwl_lut_vshort, dcc_out_prms);

        /* Parse Merge block 1 */
        tivxVpacVissParseMergeParams(0u, &rfe_prms->merge_1, dcc_out_prms);
        /* Parse Merge block 2 */
        tivxVpacVissParseMergeParams(1u, &rfe_prms->merge_2, dcc_out_prms);

        /* Parse Lut for 20 to 16 conversion */
        tivxVpacVissParseRfeLutParams(0u, &rfe_prms->lut_20to16, dcc_out_prms);

        /* Parse DPC Parameters */
        tivxVpacVissParseDpcParams(&rfe_prms->dpc, dcc_out_prms);
        /* Parse LSC Parameters */
        tivxVpacVissParseLscParams(&rfe_prms->lsc, dcc_out_prms);
        /* Parse WB2 Parameters */
        tivxVpacVissParseWB2Params(&rfe_prms->wb2, dcc_out_prms);

        /* Parse H3A Source Parameters */
        tivxVpacVissParseRfeLutParams(1u, &rfe_prms->lut_h3a, dcc_out_prms);
    }
}

void tivxVpacVissParseH3aLutParams(uint32_t idx, cfg_lut *lut,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    for (cnt = 0; cnt < PWL_LUT_SIZE; cnt ++)
    {
        lut->lut[cnt] = dcc_out_prms->issH3aMuxLuts.h3a_mux_lut[idx][cnt];
    }
}

void tivxVpacVissParseNsf4Params(nsf4_settings *nsf4_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    if (NULL != nsf4_prms)
    {
        /* DCC does not support NSF4, so using default config from
         * example_Sensor/0/0/nsf4_cfg test case */
        nsf4_prms->mode                 = 16u;
        nsf4_prms->shd_en               = 0U;
        nsf4_prms->iw                   = 1280u;
        nsf4_prms->ih                   = 720u;

        nsf4_prms->knee_u1              = 2u;
        nsf4_prms->thr_scale_tn1        = 64u;
        nsf4_prms->thr_scale_tn2        = 32u;
        nsf4_prms->thr_scale_tn3        = 16u;

        for (cnt = 0u; cnt < 4u; cnt ++)
        {
            nsf4_prms->noise_thr_x[cnt][0u]  = 0u;
            nsf4_prms->noise_thr_x[cnt][1u]  = 64u;
            nsf4_prms->noise_thr_x[cnt][2u]  = 256u;
            nsf4_prms->noise_thr_x[cnt][3u]  = 1024u;
            nsf4_prms->noise_thr_x[cnt][4u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][5u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][6u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][7u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][8u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][9u]  = 4096u;
            nsf4_prms->noise_thr_x[cnt][10u] = 4096u;
            nsf4_prms->noise_thr_x[cnt][11u] = 4096u;

            nsf4_prms->noise_thr_y[cnt][0u]  = 16u;
            nsf4_prms->noise_thr_y[cnt][1u]  = 20u;
            nsf4_prms->noise_thr_y[cnt][2u]  = 38u;
            nsf4_prms->noise_thr_y[cnt][3u]  = 76u;
            nsf4_prms->noise_thr_y[cnt][4u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][5u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][6u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][7u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][8u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][9u]  = 0u;
            nsf4_prms->noise_thr_y[cnt][10u] = 0u;
            nsf4_prms->noise_thr_y[cnt][11u] = 0u;

            nsf4_prms->noise_thr_s[cnt][0u]  = 128u;
            nsf4_prms->noise_thr_s[cnt][1u]  = 192u;
            nsf4_prms->noise_thr_s[cnt][2u]  = 100u;
            nsf4_prms->noise_thr_s[cnt][3u]  = 52u;
            nsf4_prms->noise_thr_s[cnt][4u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][5u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][6u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][7u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][8u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][9u]  = 0u;
            nsf4_prms->noise_thr_s[cnt][10u] = 0u;
            nsf4_prms->noise_thr_s[cnt][11u] = 0u;
        }

        nsf4_prms->shd_x                = 0u;
        nsf4_prms->shd_y                = 0u;
        nsf4_prms->shd_T                = 0u;
        nsf4_prms->shd_kh               = 0u;
        nsf4_prms->shd_kv               = 0u;
        nsf4_prms->shd_gmax             = 0u;
        nsf4_prms->shd_set_sel          = 0u;

        for (cnt = 0u; cnt < 2u; cnt ++)
        {
            nsf4_prms->shd_lut_x[cnt][0u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][1u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][2u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][3u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][4u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][5u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][6u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][7u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][8u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][9u]  = 0U;
            nsf4_prms->shd_lut_x[cnt][10u] = 0U;
            nsf4_prms->shd_lut_x[cnt][11u] = 0U;
            nsf4_prms->shd_lut_x[cnt][12u] = 0U;
            nsf4_prms->shd_lut_x[cnt][13u] = 0U;
            nsf4_prms->shd_lut_x[cnt][14u] = 0U;
            nsf4_prms->shd_lut_x[cnt][15u] = 0U;

            nsf4_prms->shd_lut_y[cnt][0u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][1u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][2u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][3u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][4u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][5u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][6u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][7u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][8u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][9u]  = 0U;
            nsf4_prms->shd_lut_y[cnt][10u] = 0U;
            nsf4_prms->shd_lut_y[cnt][11u] = 0U;
            nsf4_prms->shd_lut_y[cnt][12u] = 0U;
            nsf4_prms->shd_lut_y[cnt][13u] = 0U;
            nsf4_prms->shd_lut_y[cnt][14u] = 0U;
            nsf4_prms->shd_lut_y[cnt][15u] = 0U;

            nsf4_prms->shd_lut_s[cnt][0u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][1u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][2u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][3u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][4u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][5u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][6u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][7u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][8u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][9u]  = 0U;
            nsf4_prms->shd_lut_s[cnt][10u] = 0U;
            nsf4_prms->shd_lut_s[cnt][11u] = 0U;
            nsf4_prms->shd_lut_s[cnt][12u] = 0U;
            nsf4_prms->shd_lut_s[cnt][13u] = 0U;
            nsf4_prms->shd_lut_s[cnt][14u] = 0U;
            nsf4_prms->shd_lut_s[cnt][15u] = 0U;
        }

        for (cnt = 0u; cnt < 4u; cnt ++)
        {
            nsf4_prms->wb_gain[cnt] = 512U;
        }

        /* Following configuration is not exposed in registers, but the
         * cmodel requires these defaults, so setting it here. */
        nsf4_prms->lborder_rep            = 1;
        nsf4_prms->rborder_rep            = 1;
        nsf4_prms->tborder_rep            = 1;
        nsf4_prms->bborder_rep            = 1;
        nsf4_prms->supprs_all             = 0;
        nsf4_prms->bypass                 = 0;

        for (cnt = 0u; cnt < 4u; cnt ++)
        {
            nsf4_prms->supprs_max[cnt][0] = 128u;
            nsf4_prms->supprs_max[cnt][1] = 128u;
            nsf4_prms->supprs_max[cnt][2] = 128u;
            nsf4_prms->supprs_max[cnt][3] = 128u;
            nsf4_prms->supprs_max[cnt][4] = 128u;
            nsf4_prms->supprs_max[cnt][5] = 128u;
        }
        nsf4_prms->cfaw                 = 0;
        nsf4_prms->cfah                 = 0;
        nsf4_prms->ow                   = 0;
        nsf4_prms->oh                   = 0;
        nsf4_prms->ostart_x             = 0;
        nsf4_prms->ostart_y             = 0;
    }
}

void tivxVpacVissParseGlbceParams(nsf4_settings *nsf4_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
}

void tivxVpacVissParseH3aParams(h3a_settings *h3a_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (NULL != h3a_prms)
    {
        h3a_prms->pcr_AEW_EN        = dcc_out_prms->ipipeH3A_AEWBCfg.enable;
        h3a_prms->aew_cfg_AEFMT     = dcc_out_prms->ipipeH3A_AEWBCfg.mode;
        h3a_prms->aewinstart_WINSV  = dcc_out_prms->ipipeH3A_AEWBCfg.v_start;
        h3a_prms->aewinstart_WINSH  = dcc_out_prms->ipipeH3A_AEWBCfg.h_start;
        h3a_prms->aewwin1_WINH      = dcc_out_prms->ipipeH3A_AEWBCfg.v_size;
        h3a_prms->aewwin1_WINW      = dcc_out_prms->ipipeH3A_AEWBCfg.h_size;
        h3a_prms->aewwin1_WINVC     = dcc_out_prms->ipipeH3A_AEWBCfg.v_count;
        h3a_prms->aewwin1_WINHC     = dcc_out_prms->ipipeH3A_AEWBCfg.h_count;
        h3a_prms->aewsubwin_AEWINCV = dcc_out_prms->ipipeH3A_AEWBCfg.v_skip;
        h3a_prms->aewsubwin_AEWINCH = dcc_out_prms->ipipeH3A_AEWBCfg.h_skip;
        h3a_prms->pcr_AVE2LMT       = dcc_out_prms->ipipeH3A_AEWBCfg.saturation_limit;
        h3a_prms->aewinblk_WINH     = dcc_out_prms->ipipeH3A_AEWBCfg.blk_win_numlines;
        h3a_prms->aewinblk_WINSV    = dcc_out_prms->ipipeH3A_AEWBCfg.blk_row_vpos;
        h3a_prms->aew_cfg_SUMSFT    = dcc_out_prms->ipipeH3A_AEWBCfg.sum_shift;
        h3a_prms->pcr_AEW_ALAW_EN   = dcc_out_prms->ipipeH3A_AEWBCfg.ALaw_En;
        h3a_prms->pcr_AEW_MED_EN    = dcc_out_prms->ipipeH3A_AEWBCfg.MedFilt_En;
    }
}

void tivxVpacVissParseFlxCfaParams(FLXD_Config *fcfa_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt, cnt1, cnt2, cfa_cnt;
    viss_ipipe_cfa_flxd   * dcc_cfa_cfg = NULL;

    if(NULL != dcc_out_prms)
    {
        dcc_cfa_cfg = &(dcc_out_prms->vissCFACfg);
    }

    if (NULL != fcfa_prms)
    {
        if(NULL != dcc_cfa_cfg)
        {
            memcpy(fcfa_prms, dcc_cfa_cfg, sizeof(viss_ipipe_cfa_flxd));
        }
        else
        {
            fcfa_prms->imgWidth               = 1280u;
            fcfa_prms->imgHeight              = 720u;
            fcfa_prms->bitWidth               = 12u;
            fcfa_prms->lut_enable             = 0u;

            fcfa_prms->Set0GradHzMask[0u]     = 175u;
            fcfa_prms->Set0GradHzMask[1u]     = 95u;
            fcfa_prms->Set0GradHzMask[2u]     = 95u;
            fcfa_prms->Set0GradHzMask[3u]     = 175u;

            fcfa_prms->Set0GradVtMask[0u]     = 175u;
            fcfa_prms->Set0GradVtMask[1u]     = 95u;
            fcfa_prms->Set0GradVtMask[2u]     = 95u;
            fcfa_prms->Set0GradVtMask[3u]     = 175u;

            fcfa_prms->Set0IntensityMask[0u]  = 0u;
            fcfa_prms->Set0IntensityMask[1u]  = 1u;
            fcfa_prms->Set0IntensityMask[2u]  = 2u;
            fcfa_prms->Set0IntensityMask[3u]  = 3u;

            fcfa_prms->Set0IntensityShift[0u] = 4u;
            fcfa_prms->Set0IntensityShift[1u] = 5u;
            fcfa_prms->Set0IntensityShift[2u] = 6u;
            fcfa_prms->Set0IntensityShift[3u] = 7u;

            fcfa_prms->Set0Thr[0u]            = 500u;
            fcfa_prms->Set0Thr[1u]            = 600u;
            fcfa_prms->Set0Thr[2u]            = 700u;
            fcfa_prms->Set0Thr[3u]            = 800u;
            fcfa_prms->Set0Thr[4u]            = 900u;
            fcfa_prms->Set0Thr[5u]            = 1000u;
            fcfa_prms->Set0Thr[6u]            = 1100u;

            fcfa_prms->Set1GradHzMask[0u]     = 175u;
            fcfa_prms->Set1GradHzMask[1u]     = 195u;
            fcfa_prms->Set1GradHzMask[2u]     = 195u;
            fcfa_prms->Set1GradHzMask[3u]     = 175u;

            fcfa_prms->Set1GradVtMask[0u]     = 276u;
            fcfa_prms->Set1GradVtMask[1u]     = 196u;
            fcfa_prms->Set1GradVtMask[2u]     = 196u;
            fcfa_prms->Set1GradVtMask[3u]     = 276u;

            fcfa_prms->Set1IntensityMask[0u]  = 8u;
            fcfa_prms->Set1IntensityMask[1u]  = 9u;
            fcfa_prms->Set1IntensityMask[2u]  = 10u;
            fcfa_prms->Set1IntensityMask[3u]  = 11u;

            fcfa_prms->Set1IntensityShift[0u] = 12u;
            fcfa_prms->Set1IntensityShift[1u] = 13u;
            fcfa_prms->Set1IntensityShift[2u] = 14u;
            fcfa_prms->Set1IntensityShift[3u] = 15u;

            fcfa_prms->Set1Thr[0u]            = 0u;
            fcfa_prms->Set1Thr[1u]            = 100u;
            fcfa_prms->Set1Thr[2u]            = 200u;
            fcfa_prms->Set1Thr[3u]            = 300u;
            fcfa_prms->Set1Thr[4u]            = 400u;
            fcfa_prms->Set1Thr[5u]            = 500u;
            fcfa_prms->Set1Thr[6u]            = 600u;

            for (cnt = 0u; cnt < 639u; cnt ++)
            {
                fcfa_prms->ToneLut[cnt] = gcfa_lut_20to16[cnt];
            }

            cfa_cnt = 0u;
            for (cnt = 0u; cnt < 12u; cnt ++)
            {
                for (cnt1 = 0u; cnt1 < 4u; cnt1 ++)
                {
                    for (cnt2 = 0u; cnt2 < 36u; cnt2 ++)
                    {
                        fcfa_prms->FirCoefs[cnt].matrix[cnt1][cnt2] = gcfa_coeff[cfa_cnt];
                        cfa_cnt ++;
                    }
                }
            }

            fcfa_prms->blendMode[0u]          = FLXD_BLEND_SELECTHVN;
            fcfa_prms->blendMode[1u]          = FLXD_BLEND_SELECTHVN;
            fcfa_prms->blendMode[2u]          = FLXD_BLEND_SELECTHVN;
            fcfa_prms->blendMode[3u]          = FLXD_BLEND_SELECTHVN;

            fcfa_prms->bitMaskSel[0u]         = 0u;
            fcfa_prms->bitMaskSel[1u]         = 0u;
            fcfa_prms->bitMaskSel[2u]         = 0u;
            fcfa_prms->bitMaskSel[3u]         = 0u;
        }
    }
}

void tivxVpacVissParseCCMParams(Flexcc_ccm1 *ccm,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (NULL != ccm)
    {
        ccm->W11 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[0][0];
        ccm->W12 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[0][1];
        ccm->W13 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[0][2];
        ccm->W21 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[1][0];
        ccm->W22 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[1][1];
        ccm->W23 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[1][2];
        ccm->W31 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[2][0];
        ccm->W32 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[2][1];
        ccm->W33 = dcc_out_prms->ipipeRgb2Rgb1Cfg->matrix[2][2];

        ccm->Offset_1 = dcc_out_prms->ipipeRgb2Rgb1Cfg->offset[0];
        ccm->Offset_2 = dcc_out_prms->ipipeRgb2Rgb1Cfg->offset[1];
        ccm->Offset_3 = dcc_out_prms->ipipeRgb2Rgb1Cfg->offset[2];
    }
}

void tivxVpacVissParseFlxCCParams(Flexcc_Config *cc_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    if (NULL != cc_prms)
    {
        cc_prms->inWidth                    = 1280u;
        cc_prms->inHeight                   = 720u;
        cc_prms->MuxC1_4                    = 0u;
        cc_prms->MuxY12Out                  = 1u;
        cc_prms->MuxY8Out                   = 1u;
        cc_prms->MuxRGBHSV                  = 0u;
        cc_prms->HistMode                   = 0u;

        cc_prms->HistEn                     = 0u;
        cc_prms->HistStartX                 = 0u;
        cc_prms->HistStartY                 = 0u;
        cc_prms->HistSizeX                  = 500u;
        cc_prms->HistSizeY                  = 500u;

        cc_prms->CCM1.W11                   = 426;
        cc_prms->CCM1.W12                   = -166;
        cc_prms->CCM1.W13                   = -4;
        cc_prms->CCM1.W14                   = 0;
        cc_prms->CCM1.W21                   = -104;
        cc_prms->CCM1.W22                   = 353;
        cc_prms->CCM1.W23                   = 7;
        cc_prms->CCM1.W24                   = 0;
        cc_prms->CCM1.W31                   = 7;
        cc_prms->CCM1.W32                   = -178;
        cc_prms->CCM1.W33                   = 427;
        cc_prms->CCM1.W34                   = 0;

        cc_prms->CCM1.Offset_1              = 0;
        cc_prms->CCM1.Offset_2              = 0;
        cc_prms->CCM1.Offset_3              = 0;

        cc_prms->RGB2YUV.W11                = 77;
        cc_prms->RGB2YUV.W12                = 150;
        cc_prms->RGB2YUV.W13                = 29;
        cc_prms->RGB2YUV.W21                = -44;
        cc_prms->RGB2YUV.W22                = -84;
        cc_prms->RGB2YUV.W23                = 128;
        cc_prms->RGB2YUV.W31                = 128;
        cc_prms->RGB2YUV.W32                = -108;
        cc_prms->RGB2YUV.W33                = -20;

        cc_prms->RGB2YUV.Offset_1           = 0;
        cc_prms->RGB2YUV.Offset_2           = 128;
        cc_prms->RGB2YUV.Offset_3           = 128;

        cc_prms->RGB2HSV.MuxH1              = 0u;
        cc_prms->RGB2HSV.MuxH2              = 0u;
        cc_prms->RGB2HSV.GrayW11            = 64u; /* S10Q8 */
        cc_prms->RGB2HSV.GrayW12            = 64u;
        cc_prms->RGB2HSV.GrayW13            = 128u;
        cc_prms->RGB2HSV.GrayOffset_1       = 0u;

        cc_prms->RGB2HSV.SatMode            = 0u;

        cc_prms->RGB2HSV.SatDiv             = 2u;

        cc_prms->RGB2HSV.SatDivShift        = 0u; /* No Register field*/

        cc_prms->RGB2HSV.SatLutEn           = 0u;

        cc_prms->RGB2HSV.RGBLutEn           = 0u;

        cc_prms->RGB2HSV.LinLogThr_0        = 0u;
        cc_prms->RGB2HSV.LinLogThr_1        = 0u;
        cc_prms->RGB2HSV.LinLogThr_2        = 0u;
        cc_prms->RGB2HSV.Offset_0           = 0u;
        cc_prms->RGB2HSV.Offset_1           = 0u;
        cc_prms->RGB2HSV.Offset_2           = 0u;
        cc_prms->RGB2HSV.SatMinThr          = 0u;

        cc_prms->RGB2HSV.Mux_V              = 0u;

        cc_prms->ContrastEn                 = 1u;
        cc_prms->ContrastBitClip            = 10u;

        for (cnt = 0; cnt < 513; cnt ++)
        {
            cc_prms->ContrastLut[0u][cnt] = gflexcc_contrast_lut[cnt];
            cc_prms->ContrastLut[1u][cnt] = gflexcc_contrast_lut[cnt];
            cc_prms->ContrastLut[2u][cnt] = gflexcc_contrast_lut[cnt];
        }

        cc_prms->Y8LutEn                    = 0u;
        cc_prms->Y8inBitWidth               = 10u;
        cc_prms->C8LutEn                    = 0u;

        for (cnt = 0; cnt < 513; cnt ++)
        {
            cc_prms->Y8R8Lut[cnt]           = gflexcc_lut_12to8[cnt];
            cc_prms->C8G8Lut[cnt]           = gflexcc_lut_12to8[cnt];
            cc_prms->S8B8Lut[cnt]           = gflexcc_lut_12to8[cnt];
        }

        cc_prms->Y12OutEn                   = 0u;
        cc_prms->C12OutEn                   = 0u;
        cc_prms->Y8R8OutEn                  = 0u;
        cc_prms->C8G8OutEn                  = 0u;
        cc_prms->S8B8OutEn                  = 0u;


        cc_prms->ChromaMode                 = 0u;
    }
}

void tivxVpacVissParseYeeParams(ee_Config *ee_prms,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    if (NULL != ee_prms)
    {
        ee_prms->width              = 1280u;
        ee_prms->height             = 720u;
        ee_prms->yee_en             = 1u;
        ee_prms->yee_shf            = 4u;
        ee_prms->yee_mul[0][0]      = -1;
        ee_prms->yee_mul[0][1]      = -3;
        ee_prms->yee_mul[0][2]      = -5;
        ee_prms->yee_mul[0][3]      = -3;
        ee_prms->yee_mul[0][4]      = -1;
        ee_prms->yee_mul[1][0]      = -3;
        ee_prms->yee_mul[1][1]      = -2;
        ee_prms->yee_mul[1][2]      = 2;
        ee_prms->yee_mul[1][3]      = -2;
        ee_prms->yee_mul[1][4]      = -3;
        ee_prms->yee_mul[2][0]      = -5;
        ee_prms->yee_mul[2][1]      = 2;
        ee_prms->yee_mul[2][2]      = 48;
        ee_prms->yee_mul[2][3]      = 2;
        ee_prms->yee_mul[2][4]      = -5;
        ee_prms->yee_mul[3][0]      = -3;
        ee_prms->yee_mul[3][1]      = -2;
        ee_prms->yee_mul[3][2]      = 2;
        ee_prms->yee_mul[3][3]      = -2;
        ee_prms->yee_mul[3][4]      = -3;
        ee_prms->yee_mul[4][0]      = -1;
        ee_prms->yee_mul[4][1]      = -3;
        ee_prms->yee_mul[4][2]      = -5;
        ee_prms->yee_mul[4][3]      = -3;
        ee_prms->yee_mul[4][4]      = -1;
        ee_prms->yee_thr            = 0u;
        ee_prms->yes_sel            = 0u;
        ee_prms->yes_hal            = 1u;
        ee_prms->yes_g_gain         = 0u;
        ee_prms->yes_g_ofst         = 0u;
        ee_prms->yes_e_gain         = 0u;
        ee_prms->yes_e_thr1         = 0u;
        ee_prms->yes_e_thr2         = 0u;

        for (cnt = 0u; cnt < 4096u; cnt ++)
        {
            ee_prms->yee_table_s13[cnt] = gflexcc_yee_lut[cnt];
        }
    }
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static void tivxVpacVissParsePwlParams(uint32_t lut_id, cfg_pwl_lut *pwl,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    /* Long Exposure */
    if (0 == lut_id)
    {
        pwl->mask                       = 4095u;
        pwl->inShift                    = 0u;     /* 3 bits  */
        pwl->pwlEn                      = 0u;
        pwl->thrX1                      = 128u;       /* 16 bits */
        pwl->thrX2                      = 256u;
        pwl->thrX3                      = 512u;
        pwl->thrY1                      = 2048u;       /* 24 bits */
        pwl->thrY2                      = 4096u;
        pwl->thrY3                      = 8192u;
        pwl->slope1                     = 16u;      /* 16 bits */
        pwl->slope2                     = 16u;
        pwl->slope3                     = 16u;
        pwl->slope4                     = 16u;
        pwl->slopeShift                 = 0u;  /* Shift for Q point of slope */
        pwl->offset[0u]                 = 0u;   //S16
        pwl->offset[1u]                 = 0u;   //S16
        pwl->offset[2u]                 = 0u;   //S16
        pwl->offset[3u]                 = 0u;   //S16
        pwl->gain[0u]                   = 512u;     //U13Q9
        pwl->gain[1u]                   = 512u;     //U13Q9
        pwl->gain[2u]                   = 512u;     //U13Q9
        pwl->gain[3u]                   = 512u;     //U13Q9
        pwl->pwlClip                    = 65535;     /* 24 bits */
        pwl->lutEn                      = 1u;
        pwl->lutBitDepth                = 20u; /*  5 bits */
        pwl->lutClip                    = 65535;     /* 16 bits */

        for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
        {
            pwl->lut[cnt]               = grawfe_pwl_long_lut[cnt];
        }
    }
    else if (1u == lut_id) /* Short Exposure */
    {
        pwl->mask                       = 4095u;
        pwl->inShift                    = 0u;     /* 3 bits  */
        pwl->pwlEn                      = 0u;
        pwl->thrX1                      = 128u;       /* 16 bits */
        pwl->thrX2                      = 128u;
        pwl->thrX3                      = 512u;
        pwl->thrY1                      = 2048u;       /* 24 bits */
        pwl->thrY2                      = 4096u;
        pwl->thrY3                      = 8192u;
        pwl->slope1                     = 16u;      /* 16 bits */
        pwl->slope2                     = 16u;
        pwl->slope3                     = 16u;
        pwl->slope4                     = 16u;
        pwl->slopeShift                 = 0u;  /* Shift for Q point of slope */
        pwl->offset[0u]                 = 0u;   //S16
        pwl->offset[1u]                 = 0u;   //S16
        pwl->offset[2u]                 = 0u;   //S16
        pwl->offset[3u]                 = 0u;   //S16
        pwl->gain[0u]                   = 512u;     //U13Q9
        pwl->gain[1u]                   = 512u;     //U13Q9
        pwl->gain[2u]                   = 512u;     //U13Q9
        pwl->gain[3u]                   = 512u;     //U13Q9
        pwl->pwlClip                    = 65535;     /* 24 bits */
        pwl->lutEn                      = 0u;
        pwl->lutBitDepth                = 20u; /*  5 bits */
        pwl->lutClip                    = 65535;     /* 16 bits */

        for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
        {
            pwl->lut[cnt]               = grawfe_pwl_short_lut[cnt];
        }
    }
    else if (2u == lut_id) /* Very Short Exposure */
    {
        pwl->mask                       = 4095u;
        pwl->inShift                    = 0u;     /* 3 bits  */
        pwl->pwlEn                      = 0u;
        pwl->thrX1                      = 512u;       /* 16 bits */
        pwl->thrX2                      = 1408u;
        pwl->thrX3                      = 2176u;
        pwl->thrY1                      = 2048u;       /* 24 bits */
        pwl->thrY2                      = 16384u;
        pwl->thrY3                      = 65536u;
        pwl->slope1                     = 4u;      /* 16 bits */
        pwl->slope2                     = 16u;
        pwl->slope3                     = 64u;
        pwl->slope4                     = 512u;
        pwl->slopeShift                 = 0u;  /* Shift for Q point of slope */
        pwl->offset[0u]                 = -127;   //S16
        pwl->offset[1u]                 = -127;   //S16
        pwl->offset[2u]                 = -127;   //S16
        pwl->offset[3u]                 = -127;   //S16
        pwl->gain[0u]                   = 512u;     //U13Q9
        pwl->gain[1u]                   = 512u;     //U13Q9
        pwl->gain[2u]                   = 512u;     //U13Q9
        pwl->gain[3u]                   = 512u;     //U13Q9
        pwl->pwlClip                    = 1048575;     /* 24 bits */
        pwl->lutEn                      = 0u;
        pwl->lutBitDepth                = 20u; /*  5 bits */
        pwl->lutClip                    = 65535;     /* 16 bits */

        for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
        {
            pwl->lut[cnt]               = grawfe_pwl_vshort_lut[cnt];
        }
    }
    else
    {
    }
}

static void tivxVpacVissParseMergeParams(uint32_t merge_id, cfg_merge *merge,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (0U == merge_id)
    {
        merge->en                   = 0u;                 // CFG4.EN
        merge->bit_dst_u5           = 15u;         // CFG4.DST
        merge->bit_long_u4          = 4u;        // CFG4.LBIT
        merge->bit_short_u4         = 4u;       // CFG4.SBIT
        merge->wgt_sel              = 0u;            // CFG4.WGT_SEL
        merge->gain_long_u16        = 2048u;      // WDRGAIN.GLONG
        merge->gain_short_u16       = 32768u;     // WDRGAIN.SLONG

        merge->wdrbk_LBK[0u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[1u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[2u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[3u]        = 0u;       // WDRLBK
        merge->wdrbk_SBK[0u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[1u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[2u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[3u]        = 0u;       // WDRSBK
        merge->wdrwb_LWB[0u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[1u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[2u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[3u]        = 512u;       // LWDRWB
        merge->wdrwb_SWB[0u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[1u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[2u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[3u]        = 512u;       // SWDRWB

        merge->threshold_u16        = 4094u;      // WDRTHR.THR
        merge->af_m_s16             = 0u;           // WDRAF.AF_M
        merge->af_e_u5              = 0u;            // WDRAF.AF_E
        merge->bf_s16               = 0u;             // WDRBF.BF
        merge->ma_d_u16             = 65535u;           // WDRMA.MAD
        merge->ma_s_u16             = 0u;           // WDRMA.MAS
        merge->wt_sft               = 0u;             // WDRMRGCFG.MRGWTSFT
        merge->wdr_clip             = 262143u;           // WDRMRGCFG.WDRCLIP
    }
    else if (1u == merge_id)
    {
        merge->en                   = 0u;                 // CFG4.EN
        merge->bit_dst_u5           = 15u;         // CFG4.DST
        merge->bit_long_u4          = 8u;        // CFG4.LBIT
        merge->bit_short_u4         = 8u;       // CFG4.SBIT
        merge->wgt_sel              = 0u;            // CFG4.WGT_SEL
        merge->gain_long_u16        = 128u;      // WDRGAIN.GLONG
        merge->gain_short_u16       = 32768u;     // WDRGAIN.SLONG

        merge->wdrbk_LBK[0u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[1u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[2u]        = 0u;       // WDRLBK
        merge->wdrbk_LBK[3u]        = 0u;       // WDRLBK
        merge->wdrbk_SBK[0u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[1u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[2u]        = 0u;       // WDRSBK
        merge->wdrbk_SBK[3u]        = 0u;       // WDRSBK
        merge->wdrwb_LWB[0u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[1u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[2u]        = 512u;       // LWDRWB
        merge->wdrwb_LWB[3u]        = 512u;       // LWDRWB
        merge->wdrwb_SWB[0u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[1u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[2u]        = 512u;       // SWDRWB
        merge->wdrwb_SWB[3u]        = 512u;       // SWDRWB

        merge->threshold_u16        = 65504u;      // WDRTHR.THR
        merge->af_m_s16             = 0u;           // WDRAF.AF_M
        merge->af_e_u5              = 0u;            // WDRAF.AF_E
        merge->bf_s16               = 0u;             // WDRBF.BF
        merge->ma_d_u16             = 65535u;           // WDRMA.MAD
        merge->ma_s_u16             = 0u;           // WDRMA.MAS
        merge->wt_sft               = 0u;             // WDRMRGCFG.MRGWTSFT
        merge->wdr_clip             = 262143u;           // WDRMRGCFG.WDRCLIP
    }
    else
    {
    }
}

static void tivxVpacVissParseRfeLutParams(uint32_t lut_id, cfg_lut *lut,
    dcc_parser_output_params_t *dcc_out_prms)
{
    uint32_t cnt;

    /* lut for 20 to 16 bit conversion */
    if (0u == lut_id)
    {
        lut->en                 = 0u;
        lut->nbits              = 16u;
        lut->clip               = 4095u;

        for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
        {
            lut->lut[cnt]       = grawfe_lut_20to16[cnt];
        }
    }
    else if (1u == lut_id) /* H3A Lut */
    {
        if (1u == dcc_out_prms->issH3aMuxLuts.enable)
        {
            lut->en             = 1u;
            lut->nbits          = 16u;
            lut->clip           = 1023u;

            for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
            {
                lut->lut[cnt] =
                    dcc_out_prms->issH3aMuxLuts.h3a_mux_lut[0u][cnt];
            }
        }
        else
        {
            lut->en             = 0u;
        }
    }
}

static void tivxVpacVissParseDpcParams(cfg_dpc *dpc,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (NULL != dpc)
    {
        dpc->lut_en             = 0u;
        dpc->lut_bw             = 0u;
        dpc->lut_size           = 1u;

        dpc->otf_en             = 1u;
        dpc->otf_thr[0u]        = 200u;
        dpc->otf_slp[0u]        = 0u;
        dpc->otf_thr[1u]        = 200u;
        dpc->otf_slp[1u]        = 50u;
        dpc->otf_thr[2u]        = 300u;
        dpc->otf_slp[2u]        = 50u;
        dpc->otf_thr[3u]        = 500u;
        dpc->otf_slp[3u]        = 37u;
        dpc->otf_thr[4u]        = 800u;
        dpc->otf_slp[4u]        = 50u;
        dpc->otf_thr[5u]        = 1600u;
        dpc->otf_slp[5u]        = 50u;
        dpc->otf_thr[6u]        = 3200u;
        dpc->otf_slp[6u]        = 50u;
        dpc->otf_thr[7u]        = 6400u;
        dpc->otf_slp[7u]        = 50u;

        dpc->lut_vpos[0u]       = 10;
        dpc->lut_hpos[0u]       = 1494u;
        dpc->lut_method[0u]     = 0u;

        dpc->lut_vpos[1u]       = 236;
        dpc->lut_hpos[1u]       = 1107;
        dpc->lut_method[1u]     = 1u;

        dpc->lut_vpos[2u]       = 254;
        dpc->lut_hpos[2u]       = 1454;
        dpc->lut_method[2u]     = 2u;

        dpc->lut_vpos[3u]       = 444;
        dpc->lut_hpos[3u]       = 1105u;
        dpc->lut_method[3u]     = 3u;

        dpc->lut_vpos[4u]       = 488;
        dpc->lut_hpos[4u]       = 1650u;
        dpc->lut_method[4u]     = 4u;

        dpc->lut_vpos[5u]       = 772;
        dpc->lut_hpos[5u]       = 1398u;
        dpc->lut_method[5u]     = 5u;

        dpc->lut_vpos[6u]       = 870;
        dpc->lut_hpos[6u]       = 967u;
        dpc->lut_method[6u]     = 6u;

        dpc->lut_vpos[7u]       = 936;
        dpc->lut_hpos[7u]       = 712u;
        dpc->lut_method[7u]     = 7u;

        dpc->lut_vpos[8u]       = 1074;
        dpc->lut_hpos[8u]       = 358u;
        dpc->lut_method[8u]     = 7u;
    }
}

static void tivxVpacVissParseLscParams(cfg_lsc *lsc,
    dcc_parser_output_params_t *dcc_out_prms)
{
    lsc->enable                 = 0u;
    lsc->gain_mode_m            = 5u;
    lsc->gain_mode_n            = 4u;
    lsc->gain_format            = 0u;
    lsc->gain_table             = grawfe_lsc_tbl;
    lsc->gain_table_len         = 0u;
}

static void tivxVpacVissParseWB2Params(cfg_wb2 *wb,
    dcc_parser_output_params_t *dcc_out_prms)
{
    if (NULL != wb)
    {
        wb->offset[0u]              = 0u;
        wb->offset[1u]              = 0u;
        wb->offset[2u]              = 0u;
        wb->offset[3u]              = 0u;

        wb->gain[0u]                = 694u;
        wb->gain[1u]                = 512u;
        wb->gain[2u]                = 512u;
        wb->gain[3u]                = 1028u;
    }
}
