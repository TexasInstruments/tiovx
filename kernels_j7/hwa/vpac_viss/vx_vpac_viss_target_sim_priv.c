/*
 *
 * Copyright (c) 2020-2021 Texas Instruments Incorporated
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

#include "vx_vpac_viss_target_sim_priv.h"

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void tivxVpacVissParseWB2Params(cfg_wb2 *wb, Rfe_GainOfstConfig **wbCfg_p);
static void tivxVpacVissParseLscParams(cfg_lsc *lsc, Rfe_LscConfig **lscCfg_p);
static void tivxVpacVissParseDpcParams(cfg_dpc *dpc, Rfe_DpcLutConfig **dpcLutCfg_p, Rfe_DpcOtfConfig **dpcOtfCfg_p);
static void tivxVpacVissParseRfeLutParams(cfg_lut *lut, Vhwa_LutConfig **lutCfg_p);
static void tivxVpacVissParseMergeParams(cfg_merge *merge, Rfe_WdrConfig **mergeCfg_p);
static void tivxVpacVissParsePwlParams(cfg_pwl_lut *pwl, Rfe_PwlConfig **pwlCfg_p, Vhwa_LutConfig **lutCfg_p);

static void tivxVpacVissParseRfeParams(tivxVpacVissParams *prms);
static void tivxVpacVissParseH3aSrcParams(tivxVpacVissParams *prms);
static vx_status tivxVpacVissParseH3aParams(tivxVpacVissParams *prms);
static void tivxVpacVissParseNsf4Params(tivxVpacVissParams *prms);
static void tivxVpacVissParseGlbceParams(tivxVpacVissParams *prms);
static void tivxVpacVissParseFlxCfaParams(tivxVpacVissParams *prms,
    uint32_t fcp_index);
static void tivxVpacVissParseFlxCCParams(tivxVpacVissParams *prms,
    uint32_t fcp_index);
static void tivxVpacVissParseYeeParams(tivxVpacVissParams *prms,
    uint32_t fcp_index);
static void tivxVpacVissParseFcpParams(tivxVpacVissParams *prms);
static void tivxVpacVissParseCacParams(tivxVpacVissParams *prms);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissSetConfigInSim(tivxVpacVissParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxVpacVissParseRfeParams(prms);
    status = tivxVpacVissParseH3aParams(prms);
    tivxVpacVissParseCacParams(prms);
    tivxVpacVissParseNsf4Params(prms);
    tivxVpacVissParseGlbceParams(prms);
    tivxVpacVissParseFcpParams(prms);

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */


static void tivxVpacVissParseFcpParams(tivxVpacVissParams *prms)
{
    uint32_t fcp_index;

    for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
    {
        tivxVpacVissParseFlxCfaParams(prms, fcp_index);
        tivxVpacVissParseFlxCCParams(prms, fcp_index);
        tivxVpacVissParseYeeParams(prms, fcp_index);
    }
}

static void tivxVpacVissParseRfeParams(tivxVpacVissParams *prms)
{
    if (NULL != prms)
    {
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        cfg_rawfe              *rfe_prms = &prms->rawfe_params;

        rfe_prms->width             = prms->width;
        rfe_prms->height            = prms->height;
        rfe_prms->lut_shadow_en     = 0u;

        /* Parse Long exposure PWL Parameters */
        tivxVpacVissParsePwlParams(&rfe_prms->pwl_lut_long, &vissObj->vissCfgRef.lPwlCfg, &vissObj->vissCfgRef.lLutCfg);
        /* Parse Short exposure PWL Parameters */
        tivxVpacVissParsePwlParams(&rfe_prms->pwl_lut_short, &vissObj->vissCfgRef.sPwlCfg, &vissObj->vissCfgRef.sLutCfg);
        /* Parse V Short exposure PWL Parameters */
        tivxVpacVissParsePwlParams(&rfe_prms->pwl_lut_vshort, &vissObj->vissCfgRef.vsPwlCfg, &vissObj->vissCfgRef.vsLutCfg);

        /* Parse Merge block 1 */
        tivxVpacVissParseMergeParams(&rfe_prms->merge_1, &vissObj->vissCfgRef.wdr1Cfg);
        /* Parse Merge block 2 */
        tivxVpacVissParseMergeParams(&rfe_prms->merge_2, &vissObj->vissCfgRef.wdr2Cfg);

        /* Parse Lut for 20 to 16 conversion */
        tivxVpacVissParseRfeLutParams(&rfe_prms->lut_20to16, &vissObj->vissCfgRef.comp20To16LutCfg);

        /* Parse DPC Parameters */
        tivxVpacVissParseDpcParams(&rfe_prms->dpc, &vissObj->vissCfgRef.dpcLut, &vissObj->vissCfgRef.dpcOtf);
        /* Parse LSC Parameters */
        tivxVpacVissParseLscParams(&rfe_prms->lsc, &vissObj->vissCfgRef.lscCfg);
        /* Parse WB2 Parameters */
        tivxVpacVissParseWB2Params(&rfe_prms->wb2, &vissObj->vissCfgRef.wbCfg);

        /* Parse H3A Source Parameters */
        tivxVpacVissParseH3aSrcParams(prms);
        tivxVpacVissParseRfeLutParams(&rfe_prms->lut_h3a, &vissObj->vissCfgRef.h3aLutCfg);
    }
}

static void tivxVpacVissParseCacParams(tivxVpacVissParams *prms)
{
#ifdef VPAC3

    if (NULL != prms)
    {
        Cac_Config             *cac_prms = &prms->cac_params;
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Cac_Config             *cacCfg = vissCfgRef->cacCfg;

        if (NULL != cac_prms)
        {
            if (NULL != cacCfg)
            {
                int32_t i;

                memcpy(cac_prms, cacCfg, sizeof(Cac_Config));

                /* Lut is interleaved, but c model needs 4 buffers */
                for(i=0; i<2048; i++)
                {
                    prms->cac_lut[0][i] = (int8_t)((cac_prms->displacementLut[i] >>  0) & 0x00FF);
                    prms->cac_lut[1][i] = (int8_t)((cac_prms->displacementLut[i] >>  8) & 0x00FF);
                    prms->cac_lut[2][i] = (int8_t)((cac_prms->displacementLut[i] >> 16) & 0x00FF);
                    prms->cac_lut[3][i] = (int8_t)((cac_prms->displacementLut[i] >> 24) & 0x00FF);
                }

                vissCfgRef->cacCfg = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
#endif
}

static void tivxVpacVissParseNsf4Params(tivxVpacVissParams *prms)
{
    if (NULL != prms)
    {
        uint32_t cnt, cnt1, cnt2;

        nsf4_settings          *nsf4_prms = &prms->nsf4_params;
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Nsf4v_Config           *nsf4Cfg =  vissCfgRef->nsf4Cfg;
        Nsf4_LsccConfig        *lsccCfg = &nsf4Cfg->lsccCfg;

        if (NULL != nsf4_prms)
        {
            if(NULL != nsf4Cfg)
            {
#ifdef VPAC3
                /* RawHistogram */
                {
                    uint32_t i;

                    RawHistogram_Config *raw_hist_prms = &prms->raw_hist_params;
                    Nsf4_HistConfig     *rawHistCfg = &nsf4Cfg->histCfg;

                    raw_hist_prms->width = (uint16_t)prms->width;
                    raw_hist_prms->height = (uint16_t)prms->height;
                    raw_hist_prms->Lut_En = (uint8_t)rawHistCfg->histLut.enable;
                    raw_hist_prms->Lut_BitDepth = (uint8_t)rawHistCfg->inBitWidth;
                    raw_hist_prms->Phase_Select = (uint8_t)rawHistCfg->phaseSelect;

                    for(i=0; i<NSF4_HIST_MAX_ROI; i++)
                    {
                        raw_hist_prms->ROI_ENABLE[i] = rawHistCfg->roi[i].enable;
                        raw_hist_prms->ROI_START_X[i] = rawHistCfg->roi[i].start.startX;
                        raw_hist_prms->ROI_START_Y[i] = rawHistCfg->roi[i].start.startY;
                        raw_hist_prms->ROI_END_X[i] = rawHistCfg->roi[i].end.startX;
                        raw_hist_prms->ROI_END_Y[i] = rawHistCfg->roi[i].end.startY;
                    }

                    if( NULL != rawHistCfg->histLut.tableAddr )
                    {
                        for(i=0; i<HIST_LUT_SIZE; i++)
                        {
                            raw_hist_prms->Hist_Lut[i] = (uint16_t)rawHistCfg->histLut.tableAddr[i];
                        }
                    }
                }

                /* DWB */
                {
                    uint32_t i, j;

                    Nsf4_DwbConfig     *dwb_prms = &prms->dwb_params;
                    Nsf4_DwbConfig     *dwbCfg = &nsf4Cfg->dwbCfg;

                    memcpy(dwb_prms, dwbCfg, sizeof(Nsf4_DwbConfig));

                    /* Lut is interleaved, but c model needs 4 buffers */
                    for(j=0; j<FVID2_BAYER_COLOR_COMP_MAX; j++)
                    {
                        for(i=0; i<NSF4_DWB_MAX_SEGMENT; i++)
                        {
                            prms->dwb_x[j][i] = dwb_prms->dwbCurve[j][i].posX;
                            prms->dwb_y[j][i] = dwb_prms->dwbCurve[j][i].posY;
                            prms->dwb_s[j][i] = dwb_prms->dwbCurve[j][i].slope;
                        }
                    }
                }
#endif

                nsf4_prms->iw               = prms->width;
                nsf4_prms->ih               = prms->height;
                nsf4_prms->ow               = prms->width;
                nsf4_prms->oh               = prms->height;

                nsf4_prms->mode             = nsf4Cfg->mode;

                nsf4_prms->knee_u1          = nsf4Cfg->tKnee;
                nsf4_prms->thr_scale_tn1    = nsf4Cfg->tnScale[0U];
                nsf4_prms->thr_scale_tn2    = nsf4Cfg->tnScale[1U];
                nsf4_prms->thr_scale_tn3    = nsf4Cfg->tnScale[2U];

                for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
                {
                    for (cnt2 = 0U; cnt2 < NSF4_TN_MAX_SEGMENT; cnt2 ++)
                    {
                        nsf4_prms->noise_thr_x[cnt1][cnt2] = nsf4Cfg->tnCurve[cnt1][cnt2].posX;
                        nsf4_prms->noise_thr_y[cnt1][cnt2] = nsf4Cfg->tnCurve[cnt1][cnt2].posY;
                        nsf4_prms->noise_thr_s[cnt1][cnt2] = nsf4Cfg->tnCurve[cnt1][cnt2].slope;
                    }
                }

                nsf4_prms->shd_en               = lsccCfg->enable;
                nsf4_prms->shd_x                = lsccCfg->lensCenterX;
                nsf4_prms->shd_y                = lsccCfg->lensCenterY;
                nsf4_prms->shd_T                = lsccCfg->tCfg;
                nsf4_prms->shd_kh               = lsccCfg->khCfg;
                nsf4_prms->shd_kv               = lsccCfg->kvCfg;
                nsf4_prms->shd_gmax             = lsccCfg->gMaxCfg;
                nsf4_prms->shd_set_sel          = lsccCfg->setSel;

                for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
                {
                    for (cnt2 = 0U; cnt2 < NSF4_LSCC_MAX_SEGMENT; cnt2 ++)
                    {
                         nsf4_prms->shd_lut_x[cnt1][cnt2] = lsccCfg->pwlCurve[cnt1][cnt2].posX;
                         nsf4_prms->shd_lut_y[cnt1][cnt2] = lsccCfg->pwlCurve[cnt1][cnt2].posY;
                         nsf4_prms->shd_lut_s[cnt1][cnt2] = lsccCfg->pwlCurve[cnt1][cnt2].slope;
                     }
                }

                memcpy(nsf4_prms->wb_gain, nsf4Cfg->gains, 4*sizeof(int));

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

                nsf4_check_parameters(nsf4_prms);

                vissCfgRef->nsf4Cfg = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseGlbceParams(tivxVpacVissParams *prms)
{
    if (NULL != prms)
    {
        glbce_settings         *glbce_prms = &prms->glbce_params;
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Glbce_Config           *glbceCfg = vissCfgRef->glbceCfg;
        Glbce_PerceptConfig    *prcptCfg = NULL;

        if (NULL != glbce_prms)
        {
            if(NULL != glbceCfg)
            {
                glbce_prms->iw = prms->width;
                glbce_prms->ih = prms->height;

                glbce_prms->irStrength        = glbceCfg->irStrength;
                glbce_prms->intensityVariance = glbceCfg->intensityVariance;
                glbce_prms->spaceVariance     = glbceCfg->spaceVariance;
                glbce_prms->maxSlopeLimit     = glbceCfg->maxSlopeLimit;
                glbce_prms->minSlopeLimit     = glbceCfg->minSlopeLimit;

                glbce_prms->blackLevel        = glbceCfg->blackLevel;
                glbce_prms->whiteLevel        = glbceCfg->whiteLevel;
                glbce_prms->brightAmplLimit   = glbceCfg->brightAmplLimit;
                glbce_prms->darkAmplLimit     = glbceCfg->darkAmplLimit;
                glbce_prms->dither            = glbceCfg->dither;

                memcpy(glbce_prms->asymLut, glbceCfg->asymLut, GLBCE_ASYMMETRY_LUT_SIZE * sizeof(uint32_t));

                /* As per Gang's advice, WDR table isn't needed since RAWFE would already take care of it */
                glbce_prms->wdr_enable = 0;

                vissCfgRef->glbceCfg = NULL;
            }

            if(NULL != vissCfgRef->fwdPrcpCfg)
            {
                prcptCfg = &vissObj->vissCfg.fwdPrcpCfg;
                glbce_prms->fwd_percept_enable = prcptCfg->enable;
                memcpy(glbce_prms->fwd_percept_table, prcptCfg->table, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));

                vissCfgRef->fwdPrcpCfg = NULL;
            }

            if(NULL != vissCfgRef->revPrcpCfg)
            {
                prcptCfg = &vissObj->vissCfg.revPrcpCfg;
                glbce_prms->rev_percept_enable = prcptCfg->enable;
                memcpy(glbce_prms->rev_percept_table, prcptCfg->table, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));

                vissCfgRef->revPrcpCfg = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseH3aSrcParams(tivxVpacVissParams *prms)
{
    if (NULL != prms)
    {
        cfg_rawfe              *rfe_prms = &prms->rawfe_params;
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Rfe_H3aInConfig        *inCfg = vissCfgRef->rfeH3aInCfg;

        if ((NULL != rfe_prms) && (NULL != inCfg))
        {
            if(TIVX_VPAC_VISS_H3A_IN_RAW0 == inCfg->inSel)
            {
                rfe_prms->h3a_mux_sel = 2;
            }
            else if(TIVX_VPAC_VISS_H3A_IN_RAW1 == inCfg->inSel)
            {
                rfe_prms->h3a_mux_sel = 1;
            }
            else if(TIVX_VPAC_VISS_H3A_IN_RAW2 == inCfg->inSel)
            {
                rfe_prms->h3a_mux_sel = 0;
            }
            else
            {
                rfe_prms->h3a_mux_sel = 3;
            }

            rfe_prms->h3a_mux_shift = inCfg->shift;

            vissCfgRef->rfeH3aInCfg = NULL;
        }
    }
}

static vx_status tivxVpacVissParseH3aParams(tivxVpacVissParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != prms)
    {
        h3a_settings           *h3a_prms = &prms->h3a_params;
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        H3a_Config             *h3aCfg = vissCfgRef->h3aCfg;
        H3a_AewbConfig         *aewbCfg = &h3aCfg->aewbCfg;

        if ((NULL != h3a_prms) && (NULL != h3aCfg))
        {
            /* Cmodel doesn't use thse
                ??? = h3aCfg->pos.startX;
                ??? = h3aCfg->pos.startY;
            * */

            if(((aewbCfg->winCfg.height % 2u) != 0) ||
               ((aewbCfg->winCfg.width % 2u) != 0) ||
               ((aewbCfg->winCfg.vertIncr % 2u) != 0) ||
               ((aewbCfg->winCfg.horzIncr % 2u) != 0) ||
               ((aewbCfg->blackLineHeight % 2u) != 0))
            {
                VX_PRINT(VX_ZONE_ERROR, "H3A parameter not even\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                h3a_prms->pcr_AEW_EN        = (h3aCfg->module == H3A_MODULE_AEWB) ? 1u : 0u;
                h3a_prms->pcr_AEW_ALAW_EN   = aewbCfg->enableALowComp;
                h3a_prms->pcr_AEW_MED_EN    = aewbCfg->enableMedFilt;
                h3a_prms->pcr_AVE2LMT       = aewbCfg->satLimit;
                h3a_prms->aew_cfg_AEFMT     = aewbCfg->outMode;
                h3a_prms->aew_cfg_SUMSFT    = aewbCfg->sumShift;
                h3a_prms->aewinstart_WINSV  = aewbCfg->winCfg.pos.startY;
                h3a_prms->aewinstart_WINSH  = aewbCfg->winCfg.pos.startX;
                h3a_prms->aewwin1_WINH      = aewbCfg->winCfg.height;
                h3a_prms->aewwin1_WINW      = aewbCfg->winCfg.width;
                h3a_prms->aewwin1_WINVC     = aewbCfg->winCfg.vertCount;
                h3a_prms->aewwin1_WINHC     = aewbCfg->winCfg.horzCount;
                h3a_prms->aewsubwin_AEWINCV = aewbCfg->winCfg.vertIncr;
                h3a_prms->aewsubwin_AEWINCH = aewbCfg->winCfg.horzIncr;
                h3a_prms->aewinblk_WINH     = aewbCfg->blackLineHeight;
                h3a_prms->aewinblk_WINSV    = aewbCfg->blackLineVertStart;

                vissCfgRef->h3aCfg = NULL;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static void tivxVpacVissParseFlxCfaParams(tivxVpacVissParams *prms,
    uint32_t fcp_index)
{
    if (NULL != prms)
    {
        uint32_t cnt, cnt1, cnt2, cfa_cnt;

        FLXD_Config            *fcfa_prms = &prms->flexcfa_params[fcp_index];
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Fcp_CfaConfig          *cfaCfg =  vissCfgRef->fcpCfg[fcp_index].cfaCfg;
        Vhwa_LutConfig         *lut16to12Cfg = vissCfgRef->fcpCfg[fcp_index].cfaLut16to12Cfg;

        if (NULL != fcfa_prms)
        {

#ifdef VPAC3
            Fcp_comDecomLutConfig *decomLutCfg =  vissCfgRef->fcpCfg[fcp_index].decomLutCfg;
            Fcp_comDecomLutConfig *comLutCfg   =  vissCfgRef->fcpCfg[fcp_index].comLutCfg;

            if( NULL != decomLutCfg )
            {
                fcfa_prms->dCmpdLutEn = (int16_t)decomLutCfg->enable;

                for (cnt1 = 0u; cnt1 < FCP_MAX_COLOR_COMP; cnt1 ++)
                {
                    uint32_t *dCmpdLut = decomLutCfg->tableAddr[cnt1];

                    if(NULL != dCmpdLut)
                    {
                        for (cnt2 = 0u; cnt2 < FLXD_LUT_SIZE; cnt2 ++)
                        {
                            fcfa_prms->dCmpdLut[cnt1][cnt2] = dCmpdLut[cnt2];
                        }
                    }
                }
                vissCfgRef->fcpCfg[fcp_index].decomLutCfg = NULL;
            }
            if( NULL != comLutCfg )
            {
                fcfa_prms->cmpdLutEn  = (int16_t)comLutCfg->enable;

                for (cnt1 = 0u; cnt1 < FCP_MAX_COLOR_COMP; cnt1 ++)
                {
                    uint32_t *cmpdLut = comLutCfg->tableAddr[cnt1];

                    if(NULL != cmpdLut)
                    {
                        for (cnt2 = 0u; cnt2 < FLXD_LUT_SIZE; cnt2 ++)
                        {
                            fcfa_prms->cmpdLut[cnt1][cnt2]  = (uint16_t)cmpdLut[cnt2];
                        }
                    }
                }
                vissCfgRef->fcpCfg[fcp_index].comLutCfg = NULL;
            }
#endif
            if(NULL != cfaCfg)
            {
                fcfa_prms->imgWidth = prms->width;
                fcfa_prms->imgWidth = prms->height;

#ifdef VPAC3
                fcfa_prms->processMode    = (int16_t)cfaCfg->enable16BitMode;
                fcfa_prms->linearBitWidth = (int16_t)cfaCfg->linearBitWidth;
                fcfa_prms->ccmEn          = (int16_t)cfaCfg->ccmEnable;

                for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
                {
                    /* driver has extra cfaCfg->firConfig[cnt].enable that isn't in the cmodel? */
                    fcfa_prms->cfai_scalers[cnt] = (uint16_t)cfaCfg->firConfig[cnt].scaler;
                    fcfa_prms->cfai_offsets[cnt] = (uint16_t)cfaCfg->firConfig[cnt].offset;

                    fcfa_prms->CCM[cnt][0] = cfaCfg->ccmConfig[cnt].inputCh0;
                    fcfa_prms->CCM[cnt][1] = cfaCfg->ccmConfig[cnt].inputCh1;
                    fcfa_prms->CCM[cnt][2] = cfaCfg->ccmConfig[cnt].inputCh2;
                    fcfa_prms->CCM[cnt][3] = cfaCfg->ccmConfig[cnt].inputCh3;
                    fcfa_prms->CCM[cnt][4] = cfaCfg->ccmConfig[cnt].offset;
                }
#endif
                for (cnt = 0u; cnt < 4u; cnt ++)
                {
                    fcfa_prms->Set0GradHzMask[cnt]     = cfaCfg->gradHzPh[0u][cnt];
                    fcfa_prms->Set0GradVtMask[cnt]     = cfaCfg->gradVtPh[0u][cnt];
                    fcfa_prms->Set0IntensityMask[cnt]  = cfaCfg->intsBitField[0u][cnt];
                    fcfa_prms->Set0IntensityShift[cnt] = cfaCfg->intsShiftPh[0u][cnt];

                    fcfa_prms->Set1GradHzMask[cnt]     = cfaCfg->gradHzPh[1u][cnt];
                    fcfa_prms->Set1GradVtMask[cnt]     = cfaCfg->gradVtPh[1u][cnt];
                    fcfa_prms->Set1IntensityMask[cnt]  = cfaCfg->intsBitField[1u][cnt];
                    fcfa_prms->Set1IntensityShift[cnt] = cfaCfg->intsShiftPh[1u][cnt];

                    fcfa_prms->blendMode[cnt]          = cfaCfg->coreBlendMode[cnt];
                    fcfa_prms->bitMaskSel[cnt]         = cfaCfg->coreSel[cnt];
                }

                for (cnt = 0u; cnt < 7u; cnt ++)
                {
                    fcfa_prms->Set0Thr[cnt]     = cfaCfg->thr[0u][cnt];
                    fcfa_prms->Set1Thr[cnt]     = cfaCfg->thr[1u][cnt];
                }

                cfa_cnt = 0u;
                for (cnt = 0u; cnt < 12u; cnt ++)
                {
                    for (cnt1 = 0u; cnt1 < 4u; cnt1 ++)
                    {
                        for (cnt2 = 0u; cnt2 < 36u; cnt2 ++)
                        {
                            fcfa_prms->FirCoefs[cnt].matrix[cnt1][cnt2] = cfaCfg->coeff[cfa_cnt];
                            cfa_cnt ++;
                        }
                    }
                }

                vissCfgRef->fcpCfg[fcp_index].cfaCfg = NULL;
            }

            if(NULL != lut16to12Cfg)
            {
                fcfa_prms->bitWidth               = lut16to12Cfg->inputBits;
                fcfa_prms->lut_enable             = lut16to12Cfg->enable;

                for (cnt = 0u; cnt < 639u; cnt ++)
                {
                    fcfa_prms->ToneLut[cnt] = lut16to12Cfg->tableAddr[cnt];
                }

                vissCfgRef->fcpCfg[fcp_index].cfaLut16to12Cfg = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseFlxCCParams(tivxVpacVissParams *prms,
    uint32_t fcp_index)
{
    if (NULL != prms)
    {
        uint32_t cnt;

        Flexcc_Config          *cc_prms = &prms->flexcc_params[fcp_index];
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Fcp_CcmConfig          *ccmCfg =  vissCfgRef->fcpCfg[fcp_index].ccm;
        Fcp_Rgb2YuvConfig      *r2y = vissCfgRef->fcpCfg[fcp_index].rgb2yuv;
        Fcp_Rgb2HsvConfig      *r2h = vissCfgRef->fcpCfg[fcp_index].rgb2Hsv;
        Fcp_GammaConfig        *gamma = vissCfgRef->fcpCfg[fcp_index].gamma;
        Fcp_YuvSatLutConfig    *yuvSatLutCfg = vissCfgRef->fcpCfg[fcp_index].yuvSatLutCfg;
        Fcp_HistConfig         *histCfg = vissCfgRef->fcpCfg[fcp_index].histCfg;

        if (NULL != cc_prms)
        {
            cc_prms->inWidth = prms->width;
            cc_prms->inHeight = prms->height;

            cc_prms->MuxC1_4                    = 0u; /* JGV No mapping to drv? */
            cc_prms->MuxY12Out                  = 1u; /* JGV No mapping to drv? */
            cc_prms->MuxY8Out                   = 1u; /* JGV No mapping to drv? */
            //cc_prms->ChromaMode                 = 0u; /* JGV Set externally in node */
#ifdef VPAC3 // VPAC3 CC for MV
            if (fcp_index > 0)
            {
                cc_prms->MuxC1_4                = 3u; /* JGV No mapping to drv? */
                cc_prms->MuxY12Out              = 0u; /* JGV No mapping to drv? */
                cc_prms->MuxY8Out               = 0u; /* JGV No mapping to drv? */
            }
#endif
            if (NULL != ccmCfg)
            {
                cc_prms->CCM1.W11                   = ccmCfg->weights[0][0];
                cc_prms->CCM1.W12                   = ccmCfg->weights[0][1];
                cc_prms->CCM1.W13                   = ccmCfg->weights[0][2];
                cc_prms->CCM1.W14                   = ccmCfg->weights[0][3];
                cc_prms->CCM1.W21                   = ccmCfg->weights[1][0];
                cc_prms->CCM1.W22                   = ccmCfg->weights[1][1];
                cc_prms->CCM1.W23                   = ccmCfg->weights[1][2];
                cc_prms->CCM1.W24                   = ccmCfg->weights[1][3];
                cc_prms->CCM1.W31                   = ccmCfg->weights[2][0];
                cc_prms->CCM1.W32                   = ccmCfg->weights[2][1];
                cc_prms->CCM1.W33                   = ccmCfg->weights[2][2];
                cc_prms->CCM1.W34                   = ccmCfg->weights[2][3];

                cc_prms->CCM1.Offset_1              = ccmCfg->offsets[0];
                cc_prms->CCM1.Offset_2              = ccmCfg->offsets[1];
                cc_prms->CCM1.Offset_3              = ccmCfg->offsets[2];

                vissCfgRef->fcpCfg[fcp_index].ccm = NULL;
            }

            if (NULL != r2y)
            {
                cc_prms->RGB2YUV.W11                = r2y->weights[0u][0u];
                cc_prms->RGB2YUV.W12                = r2y->weights[0u][1u];
                cc_prms->RGB2YUV.W13                = r2y->weights[0u][2u];
                cc_prms->RGB2YUV.W21                = r2y->weights[1u][0u];
                cc_prms->RGB2YUV.W22                = r2y->weights[1u][1u];
                cc_prms->RGB2YUV.W23                = r2y->weights[1u][2u];
                cc_prms->RGB2YUV.W31                = r2y->weights[2u][0u];
                cc_prms->RGB2YUV.W32                = r2y->weights[2u][1u];
                cc_prms->RGB2YUV.W33                = r2y->weights[2u][2u];

                cc_prms->RGB2YUV.Offset_1           = r2y->offsets[0u];
                cc_prms->RGB2YUV.Offset_2           = r2y->offsets[1u];
                cc_prms->RGB2YUV.Offset_3           = r2y->offsets[2u];

                vissCfgRef->fcpCfg[fcp_index].rgb2yuv = NULL;
            }

            if (NULL != r2h)
            {
                cc_prms->RGB2HSV.MuxH1              = r2h->h1Input;
                cc_prms->RGB2HSV.MuxH2              = r2h->h2Input;
                cc_prms->RGB2HSV.GrayW11            = r2h->weights[0u]; /* S10Q8 */
                cc_prms->RGB2HSV.GrayW12            = r2h->weights[1u];
                cc_prms->RGB2HSV.GrayW13            = r2h->weights[2u];
                cc_prms->RGB2HSV.GrayOffset_1       = r2h->offset;

                cc_prms->RGB2HSV.SatMode            = r2h->satMode;

                cc_prms->RGB2HSV.SatDiv             = r2h->satDiv;

                cc_prms->RGB2HSV.SatDivShift        = 0u; /* No Register field*/

                cc_prms->RGB2HSV.SatLutEn           = 0u; /* JGV No mapping to drv? */

                cc_prms->RGB2HSV.RGBLutEn           = 0u; /* JGV No mapping to drv? */

                cc_prms->RGB2HSV.LinLogThr_0        = r2h->threshold[0u];
                cc_prms->RGB2HSV.LinLogThr_1        = r2h->threshold[1u];
                cc_prms->RGB2HSV.LinLogThr_2        = r2h->threshold[2u];
                cc_prms->RGB2HSV.Offset_0           = r2h->wbOffset[0u];
                cc_prms->RGB2HSV.Offset_1           = r2h->wbOffset[1u];
                cc_prms->RGB2HSV.Offset_2           = r2h->wbOffset[2u];
                cc_prms->RGB2HSV.SatMinThr          = r2h->satMinThr;

                cc_prms->RGB2HSV.Mux_V              = r2h->useWbDataForGreyCalc;

                cc_prms->MuxRGBHSV                  = r2h->inputSelect;

                vissCfgRef->fcpCfg[fcp_index].rgb2Hsv = NULL;
            }

            if (NULL != gamma)
            {
                cc_prms->ContrastEn                 = gamma->enable;
                cc_prms->ContrastBitClip            = gamma->outClip;

                for (cnt = 0; cnt < 513; cnt ++)
                {
                    cc_prms->ContrastLut[0u][cnt] = gamma->tableC1[cnt];
                    cc_prms->ContrastLut[1u][cnt] = gamma->tableC2[cnt];
                    cc_prms->ContrastLut[2u][cnt] = gamma->tableC3[cnt];
                }

                vissCfgRef->fcpCfg[fcp_index].gamma = NULL;
            }

            if (NULL != yuvSatLutCfg)
            {
                cc_prms->Y8LutEn                    = yuvSatLutCfg->enableLumaLut;
                cc_prms->Y8inBitWidth               = yuvSatLutCfg->lumaInputBits;
                cc_prms->C8LutEn                    = yuvSatLutCfg->enableChromaLut;
                                                      //yuvSatLutCfg->enableSaturLut = (uint32_t) FALSE; /* JGV No mapping to sim? */

                for (cnt = 0; cnt < 513; cnt ++)
                {
                    cc_prms->Y8R8Lut[cnt]           = yuvSatLutCfg->lumaLutAddr[cnt];
                    cc_prms->C8G8Lut[cnt]           = yuvSatLutCfg->chromaLutAddr[cnt];
                    cc_prms->S8B8Lut[cnt]           = yuvSatLutCfg->saturLutAddr[cnt];
                }

                vissCfgRef->fcpCfg[fcp_index].yuvSatLutCfg = NULL;
            }

            if (NULL != histCfg)
            {
                cc_prms->HistEn                     = histCfg->enable;
                cc_prms->HistMode                   = histCfg->input;
                cc_prms->HistStartX                 = histCfg->roi.cropStartX;
                cc_prms->HistStartY                 = histCfg->roi.cropStartY;
                cc_prms->HistSizeX                  = histCfg->roi.cropWidth;
                cc_prms->HistSizeY                  = histCfg->roi.cropHeight;

                vissCfgRef->fcpCfg[fcp_index].histCfg = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseYeeParams(tivxVpacVissParams *prms,
    uint32_t fcp_index)
{
    if (NULL != prms)
    {
        ee_Config              *ee_prms = &prms->ee_params[fcp_index];
        tivxVpacVissObj        *vissObj = &prms->vissObj;
        tivxVpacVissConfigRef  *vissCfgRef = &vissObj->vissCfgRef;
        Fcp_EeConfig           *eeCfg = vissCfgRef->fcpCfg[fcp_index].eeCfg;

        if ((NULL != ee_prms) && (NULL != eeCfg))
        {
            uint32_t cnt;

            /* CMODEL doesn't support, these are handled by node muxing logic from node config params
            eeCfg->alignY12withChroma = FALSE;
            eeCfg->alignY8withChroma = FALSE;
            eeCfg->eeForY12OrY8 = 0;
            eeCfg->bypassY12 = FALSE;
            eeCfg->bypassC12 = TRUE;
            eeCfg->bypassY8 = TRUE;
            eeCfg->bypassC8 = TRUE;
            eeCfg->leftShift = 0;
            eeCfg->rightShift = 0;
            * */

            ee_prms->width             = prms->width;
            ee_prms->height            = prms->height;

            ee_prms->yee_en             = eeCfg->enable;
            ee_prms->yee_shf            = eeCfg->yeeShift;
            ee_prms->yee_mul[0][0]      = eeCfg->coeff[0];
            ee_prms->yee_mul[0][1]      = eeCfg->coeff[1];
            ee_prms->yee_mul[0][2]      = eeCfg->coeff[2];
            ee_prms->yee_mul[0][3]      = eeCfg->coeff[1];
            ee_prms->yee_mul[0][4]      = eeCfg->coeff[0];
            ee_prms->yee_mul[1][0]      = eeCfg->coeff[3];
            ee_prms->yee_mul[1][1]      = eeCfg->coeff[4];
            ee_prms->yee_mul[1][2]      = eeCfg->coeff[5];
            ee_prms->yee_mul[1][3]      = eeCfg->coeff[4];
            ee_prms->yee_mul[1][4]      = eeCfg->coeff[3];
            ee_prms->yee_mul[2][0]      = eeCfg->coeff[6];
            ee_prms->yee_mul[2][1]      = eeCfg->coeff[7];
            ee_prms->yee_mul[2][2]      = eeCfg->coeff[8];
            ee_prms->yee_mul[2][3]      = eeCfg->coeff[7];
            ee_prms->yee_mul[2][4]      = eeCfg->coeff[6];
            ee_prms->yee_mul[3][0]      = eeCfg->coeff[3];
            ee_prms->yee_mul[3][1]      = eeCfg->coeff[4];
            ee_prms->yee_mul[3][2]      = eeCfg->coeff[5];
            ee_prms->yee_mul[3][3]      = eeCfg->coeff[4];
            ee_prms->yee_mul[3][4]      = eeCfg->coeff[3];
            ee_prms->yee_mul[4][0]      = eeCfg->coeff[0];
            ee_prms->yee_mul[4][1]      = eeCfg->coeff[1];
            ee_prms->yee_mul[4][2]      = eeCfg->coeff[2];
            ee_prms->yee_mul[4][3]      = eeCfg->coeff[1];
            ee_prms->yee_mul[4][4]      = eeCfg->coeff[0];

            ee_prms->yee_thr            = eeCfg->yeeEThr;
            ee_prms->yes_sel            = eeCfg->yeeMergeSel;
            ee_prms->yes_hal            = eeCfg->haloReductionOn;
            ee_prms->yes_g_gain         = eeCfg->yesGGain;
            ee_prms->yes_g_ofst         = eeCfg->yesGOfset;
            ee_prms->yes_e_gain         = eeCfg->yesEGain;
            ee_prms->yes_e_thr1         = eeCfg->yesEThr1;
            ee_prms->yes_e_thr2         = eeCfg->yesEThr2;

            for (cnt = 0u; cnt < 4096u; cnt ++)
            {
                ee_prms->yee_table_s13[cnt] = eeCfg->lut[cnt];
            }

            vissCfgRef->fcpCfg[fcp_index].eeCfg = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParsePwlParams(cfg_pwl_lut *pwl, Rfe_PwlConfig **pwlCfg_p, Vhwa_LutConfig **lutCfg_p)
{
    if ((NULL != pwl) && (NULL != pwlCfg_p) && (NULL != lutCfg_p))
    {
        uint32_t cnt;

        Rfe_PwlConfig *pwlCfg = *pwlCfg_p;
        Vhwa_LutConfig *lutCfg = *lutCfg_p;

        if(NULL != pwlCfg)
        {
            pwl->mask                       = pwlCfg->mask;
            pwl->inShift                    = pwlCfg->shift;        /* 3 bits  */
            pwl->pwlEn                      = pwlCfg->enable;
            pwl->thrX1                      = pwlCfg->xthr1;       /* 16 bits */
            pwl->thrX2                      = pwlCfg->xthr2;
            pwl->thrX3                      = pwlCfg->xthr3;
            pwl->thrY1                      = pwlCfg->ythr1;       /* 24 bits */
            pwl->thrY2                      = pwlCfg->ythr2;
            pwl->thrY3                      = pwlCfg->ythr3;
            pwl->slope1                     = pwlCfg->slope1;      /* 16 bits */
            pwl->slope2                     = pwlCfg->slope2;
            pwl->slope3                     = pwlCfg->slope3;
            pwl->slope4                     = pwlCfg->slope4;
            pwl->slopeShift                 = pwlCfg->slopeShift;  /* Shift for Q point of slope */
            pwl->pwlClip                    = pwlCfg->outClip;     /* 24 bits */

            for (cnt = 0u; cnt < RFE_MAX_COLOR_COMP; cnt ++)
            {
                pwl->offset[cnt]                 = pwlCfg->offset[cnt];   //S16
                pwl->gain[cnt]                   = pwlCfg->gain[cnt];     //U13Q9
            }

            *pwlCfg_p = NULL;
        }

        if(NULL != lutCfg)
        {
            pwl->lutEn                      = lutCfg->enable;
            pwl->lutBitDepth                = lutCfg->inputBits; /*  5 bits */
            pwl->lutClip                    = lutCfg->clip;      /* 16 bits */

            for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
            {
                pwl->lut[cnt]               = lutCfg->tableAddr[cnt];
            }

            *lutCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseMergeParams(cfg_merge *merge, Rfe_WdrConfig **mergeCfg_p)
{
    if ((NULL != merge) && (NULL != mergeCfg_p))
    {
        Rfe_WdrConfig *mergeCfg = *mergeCfg_p;

        if(NULL != mergeCfg)
        {
            uint32_t cnt;

            merge->en                   = mergeCfg->enable;                 // CFG4.EN
            merge->bit_dst_u5           = mergeCfg->dst;         // CFG4.DST
            merge->bit_long_u4          = mergeCfg->lbit;        // CFG4.LBIT
            merge->bit_short_u4         = mergeCfg->sbit;       // CFG4.SBIT
            merge->wgt_sel              = mergeCfg->useShortExpForWgtCalc;            // CFG4.WGT_SEL
            merge->gain_long_u16        = mergeCfg->glong;      // WDRGAIN.GLONG
            merge->gain_short_u16       = mergeCfg->gshort;     // WDRGAIN.SLONG

            for (cnt = 0u; cnt < RFE_MAX_COLOR_COMP; cnt ++)
            {
                merge->wdrbk_LBK[cnt] = mergeCfg->lbk[cnt];
                merge->wdrbk_SBK[cnt] = mergeCfg->sbk[cnt];
                merge->wdrwb_LWB[cnt] = mergeCfg->lwb[cnt];
                merge->wdrwb_SWB[cnt] = mergeCfg->swb[cnt];
            }

            merge->threshold_u16        = mergeCfg->wdrThr;      // WDRTHR.THR
            merge->af_m_s16             = mergeCfg->afm;         // WDRAF.AF_M
            merge->af_e_u5              = mergeCfg->afe;         // WDRAF.AF_E
            merge->bf_s16               = mergeCfg->bf;          // WDRBF.BF
            merge->ma_d_u16             = mergeCfg->mad;         // WDRMA.MAD
            merge->ma_s_u16             = mergeCfg->mas;         // WDRMA.MAS
            merge->wt_sft               = mergeCfg->mergeShift;  // WDRMRGCFG.MRGWTSFT
            merge->wdr_clip             = mergeCfg->mergeClip;   // WDRMRGCFG.WDRCLIP

            *mergeCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseRfeLutParams(cfg_lut *lut, Vhwa_LutConfig **lutCfg_p)
{
    if ((NULL != lut) && (NULL != lutCfg_p))
    {
        Vhwa_LutConfig *lutCfg = *lutCfg_p;

        if(NULL != lutCfg)
        {
            uint32_t cnt;

            lut->en                 = lutCfg->enable;
            lut->nbits              = lutCfg->inputBits;
            lut->clip               = lutCfg->clip;

            if(lut->en != 0)
            {
                for (cnt = 0u; cnt < PWL_LUT_SIZE; cnt ++)
                {
                    lut->lut[cnt]       = lutCfg->tableAddr[cnt];
                }
            }

            *lutCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseDpcParams(cfg_dpc *dpc, Rfe_DpcLutConfig **dpcLutCfg_p, Rfe_DpcOtfConfig **dpcOtfCfg_p)
{
    if ((NULL != dpc) && (NULL != dpcLutCfg_p) && (NULL != dpcOtfCfg_p))
    {
        uint32_t cnt;

        Rfe_DpcLutConfig *dpcLutCfg = *dpcLutCfg_p;
        Rfe_DpcOtfConfig *dpcOtfCfg = *dpcOtfCfg_p;

        if(NULL != dpcLutCfg)
        {
            dpc->lut_en             = dpcLutCfg->enable;
            dpc->lut_bw             = dpcLutCfg->isReplaceWhite;
            dpc->lut_size           = dpcLutCfg->maxDefectPixels;

            /* TODO: Driver not set to same as default */
            for (cnt = 0u; cnt < dpc->lut_size; cnt ++)
            {
                dpc->lut_hpos[cnt]       = dpcLutCfg->table[3*cnt+0];
                dpc->lut_vpos[cnt]       = dpcLutCfg->table[3*cnt+1];
                dpc->lut_method[cnt]     = dpcLutCfg->table[3*cnt+2];
            }
            *dpcLutCfg_p = NULL;
        }

        if(NULL != dpcOtfCfg)
        {
            dpc->otf_en                 = dpcOtfCfg->enable;
            for (cnt = 0u; cnt < RFE_DPC_OTF_LUT_SIZE; cnt ++)
            {
                dpc->otf_thr[cnt]       = dpcOtfCfg->threshold[cnt];
                dpc->otf_slp[cnt]       = dpcOtfCfg->slope[cnt];
            }
            *dpcOtfCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseLscParams(cfg_lsc *lsc, Rfe_LscConfig **lscCfg_p)
{
    if ((NULL != lsc) && (NULL != lscCfg_p))
    {
        Rfe_LscConfig *lscCfg = *lscCfg_p;

        if(NULL != lscCfg)
        {
            uint32_t cnt;

            lsc->enable                 = lscCfg->enable;
            lsc->gain_mode_m            = lscCfg->horzDsFactor;
            lsc->gain_mode_n            = lscCfg->vertDsFactor;
            lsc->gain_format            = lscCfg->gainFmt;
            lsc->gain_table_len         = lscCfg->numTblEntry*4;

            if (lsc->gain_table_len > 0)
            {
                if(NULL != lsc->gain_table)
                {
                    free(lsc->gain_table);
                }

                lsc->gain_table = malloc(lsc->gain_table_len*sizeof(int));

                if (NULL == lsc->gain_table)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Can't alloc LSC gain table of size = %d bytes\n", lsc->gain_table_len*sizeof(int));
                }
                else
                {
                    uint8_t *tblPtr = (uint8_t*)lscCfg->tableAddr;
                    for (cnt = 0u; cnt < lsc->gain_table_len; cnt ++)
                    {
                        lsc->gain_table[cnt] = (int)tblPtr[cnt];
                    }
                }
            }

            *lscCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}

static void tivxVpacVissParseWB2Params(cfg_wb2 *wb, Rfe_GainOfstConfig **wbCfg_p)
{
    if ((NULL != wb) && (NULL != wbCfg_p))
    {
        Rfe_GainOfstConfig *wbCfg = *wbCfg_p;

        if(NULL != wbCfg)
        {
            uint32_t cnt;

            for (cnt = 0u; cnt < 4; cnt ++)
            {
                wb->gain[cnt]   = wbCfg->gain[cnt];
                wb->offset[cnt] = wbCfg->offset[cnt];
            }
            *wbCfg_p = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL pointer\n");
    }
}
