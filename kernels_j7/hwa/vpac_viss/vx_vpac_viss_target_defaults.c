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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <vx_vpac_viss_target_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void tivxVpacVissDefaultMapH3aParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDefaultMapNsf4Params(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDefaultMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDefaultMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index);
static void tivxVpacVissDefaultMapRGB2YUVParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapRGB2HSVParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapGammaParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapRGBLutParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapHistParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapFcpParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapFlexCFAParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapGlbceParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapDpcLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapDpcOtfParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapMergeParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);
static void tivxVpacVissDefaultMapLscParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapRfeLutParams(tivxVpacVissObj *vissObj,
    uint32_t lut_id);
static void tivxVpacVissDefaultMapWb2Params(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);
static void tivxVpacVissDefaultMapRfeParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapH3aLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapFlexCCParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapEeParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDefaultMapCacParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDefaultMapPcidParams(tivxVpacVissObj *vissObj);

/***************************************************
 * Function call heirarchy in this file:
 *
tivxVpacVissSetDefaultParams
- tivxVpacVissDefaultMapRfeParams
  - tivxVpacVissDefaultMapPwlParams (3)
  - tivxVpacVissDefaultMapMergeParams (2)
  - tivxVpacVissDefaultMapRfeLutParams (2)
  - tivxVpacVissDefaultMapDpcOtfParams
  - tivxVpacVissDefaultMapDpcLutParams
  - tivxVpacVissDefaultMapLscParams
  - tivxVpacVissDefaultMapWb2Params
- tivxVpacVissDefaultMapCacParams
- tivxVpacVissDefaultMapNsf4Params
- tivxVpacVissDefaultMapH3aParams
- tivxVpacVissDefaultMapH3aLutParams
- tivxVpacVissDefaultMapGlbceParams
- tivxVpacVissDefaultMapFcpParams
  - tivxVpacVissDefaultMapFlexCFAParams
  - tivxVpacVissDefaultMapFlexCCParams
    - tivxVpacVissDefaultMapCCMParams(vissObj, NULL);
    - tivxVpacVissDefaultMapRGB2YUVParams(vissObj);
    - tivxVpacVissDefaultMapRGB2HSVParams(vissObj);
    - tivxVpacVissDefaultMapGammaParams(vissObj);
    - tivxVpacVissDefaultMapRGBLutParams(vissObj);
    - tivxVpacVissDefaultMapHistParams(vissObj);
    - tivxVpacVissDefaultMapEeParams(vissObj);
- tivxVpacVissDefaultMapBlc
- tivxVpacVissDefaultMapPcidParams

****************************************************/

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Creating LUT array here, so that only one copy is allocated for all handles
 * Once the DCC supports direct write to Driver structures, this should
 * be removed
 */
static uint32_t gcfa_lut_16to12[] =
{
    #include "flexcfa_lut_20to16_0.txt"
};

static int32_t gcfa_coeff[] =
{
    #include "flexcfa_cfa_0.txt"
};

static uint32_t gflexcc_contrast_lut[] =
{
    #include "flexcc_contrast_lut_0.txt"
};

static uint32_t gflexcc_lut_12to8[] =
{
    #include "flexcc_lut_12to8_0.txt"
};

static uint32_t grawfe_lut_20to16[] =
{
    #include "rawfe_lut_20to16_0.txt"
};

static uint32_t grawfe_lsc_tbl[] =
{
    #include "rawfe_lsc_tbl_0.txt"
};

static uint32_t grawfe_pwl_long_lut[] =
{
    #include "rawfe_pwl_lut_long_0.txt"
};

static uint32_t grawfe_pwl_short_lut[] =
{
    #include "rawfe_pwl_lut_short_0.txt"
};

static uint32_t grawfe_pwl_vshort_lut[] =
{
    #include "rawfe_pwl_lut_vshort_0.txt"
};

static uint32_t gGlbceAsymTbl[] =
{
    0,12173,20997,27687,32934,37159,40634,43543,46014,48138,49984,
    51603,53035,54310,55453,56483,57416,58265,59041,59753,60409,
    61015,61577,62099,62585,63039,63464,63863,64237,64590,64923,65237,65535,
};

static uint32_t gGlbceFwdPrcptTbl[] =
{
    #include "glbce_fwd_percept_lut.txt"
};

static uint32_t gGlbceRevPrcptTbl[] =
{
    #include "glbce_rev_percept_lut.txt"
};

static int32_t yee_lut[] =
{
    #include "yee_lut.txt"
};

#if defined (VPAC3) || defined (VPAC3L) 
static int32_t dwb_lut[] =
{
    #include "dwb_lut.txt"
};

static int32_t cac_lut[CAC_LUT_SIZE] = {0};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissSetDefaultParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms, tivx_ae_awb_params_t *ae_awb_res)
{
    vx_status                   status = (vx_status)VX_SUCCESS;

    if (NULL != vissObj)
    {
        vissObj->bypass_nsf4 = vissPrms->bypass_nsf4;

        tivxVpacVissDefaultMapRfeParams(vissObj);
        tivxVpacVissDefaultMapCacParams(vissObj);
        tivxVpacVissDefaultMapNsf4Params(vissObj, ae_awb_res);
        if ((vx_bool)vx_true_e == vissObj->h3a_out_enabled)
        {
            tivxVpacVissDefaultMapH3aParams(vissObj, ae_awb_res);
        }
        tivxVpacVissDefaultMapH3aLutParams(vissObj);
        tivxVpacVissDefaultMapGlbceParams(vissObj);
        tivxVpacVissDefaultMapFcpParams(vissObj);
        tivxVpacVissDefaultMapBlc(vissObj, ae_awb_res);
        tivxVpacVissDefaultMapPcidParams(vissObj);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vissObj is NULL !!!\n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static void tivxVpacVissDefaultMapH3aLutParams(tivxVpacVissObj *vissObj)
{
    Vhwa_LutConfig     *h3aLutCfg = NULL;

    if (NULL != vissObj)
    {
        h3aLutCfg = &vissObj->vissCfg.h3aLutCfg;

        h3aLutCfg->enable = (uint32_t)FALSE;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;

        vissObj->vissCfgRef.h3aLutCfg = h3aLutCfg;
    }
}

static void tivxVpacVissDefaultMapH3aParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    H3a_Config         *h3aCfg = NULL;
    H3a_AewbConfig     *aewbCfg = NULL;

    if (NULL != vissObj)
    {
        h3aCfg = &vissObj->vissCfg.h3aCfg;
        aewbCfg = &h3aCfg->aewbCfg;

        /* MAP DCC config to FVID2 driver config */
        h3aCfg->module                  = H3A_MODULE_AEWB;
        h3aCfg->pos.startX              = 0u;
        h3aCfg->pos.startY              = 0u;
        aewbCfg->enableALowComp         = 0u;
        aewbCfg->enableMedFilt          = 0u;
        /* TODO: h3aCfg->midFiltThreshold = */
        aewbCfg->winCfg.pos.startX      = 0u;
        aewbCfg->winCfg.pos.startY      = 0u;
        aewbCfg->winCfg.width           = 0u;
        aewbCfg->winCfg.height          = 0u;
        aewbCfg->winCfg.horzCount       = 0u;
        aewbCfg->winCfg.vertCount       = 0u;
        aewbCfg->winCfg.horzIncr        = 0u;
        aewbCfg->winCfg.vertIncr        = 0u;
        aewbCfg->blackLineVertStart     = 0u;
        aewbCfg->blackLineHeight        = 0u;
        aewbCfg->outMode                = 0u;
        aewbCfg->sumShift               = 0u;
        aewbCfg->satLimit               = 0u;

        vissObj->vissCfgRef.h3aCfg = h3aCfg;

        vissObj->aew_config.aewwin1_WINH = 0u;
        vissObj->aew_config.aewwin1_WINW = 0u;
        vissObj->aew_config.aewwin1_WINVC = 0u;
        vissObj->aew_config.aewwin1_WINHC = 0u;
        vissObj->aew_config.aewsubwin_AEWINCV = 0u;
        vissObj->aew_config.aewsubwin_AEWINCH = 0u;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapNsf4Params(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t            cnt1, cnt2;
    Nsf4v_Config       *nsf4Cfg = NULL;
    Nsf4_LsccConfig    *lsccCfg = NULL;

#if defined(VPAC3) || defined(VPAC3L)
    Nsf4_HistConfig    *rawHistCfg = NULL;
    Nsf4_DwbConfig     *dwbCfg = NULL;
#endif

    /* TODO: Does shading gain map to lscc enable? */

    if (NULL != vissObj)
    {
        nsf4Cfg = &vissObj->vissCfg.nsf4Cfg;
        lsccCfg = &nsf4Cfg->lsccCfg;

#if defined(VPAC3) || defined(VPAC3L)
        rawHistCfg = &nsf4Cfg->histCfg;
        dwbCfg     = &nsf4Cfg->dwbCfg;

        dwbCfg->enable = 0;
        memcpy(dwbCfg->dwbCurve, dwb_lut, sizeof(dwbCfg->dwbCurve));

        dwbCfg->dwbLineWeights[0][0] = 0;
        dwbCfg->dwbLineWeights[0][1] = 0;
        dwbCfg->dwbLineWeights[0][2] = 0;
        dwbCfg->dwbLineWeights[0][3] = 256U;
        dwbCfg->dwbLineWeights[0][4] = 0;
        dwbCfg->dwbLineWeights[0][5] = 0;

        dwbCfg->dwbLineWeights[1][0] = 0;
        dwbCfg->dwbLineWeights[1][1] = 0;
        dwbCfg->dwbLineWeights[1][2] = 256U;
        dwbCfg->dwbLineWeights[1][3] = 0;
        dwbCfg->dwbLineWeights[1][4] = 0;
        dwbCfg->dwbLineWeights[1][5] = 0;

        rawHistCfg->enable = 0;
        rawHistCfg->inBitWidth = 12;
        rawHistCfg->phaseSelect = 0;
        memset(rawHistCfg->roi, 0 ,sizeof(Nsf4_HistRoi));
        memset(&rawHistCfg->histLut, 0 ,sizeof(Nsf4_HistLutConfig));
#endif

        nsf4Cfg->mode = 16u;
        nsf4Cfg->tKnee = 0u;
        nsf4Cfg->tnScale[0U] = 64u;
        nsf4Cfg->tnScale[1U] = 32u;
        nsf4Cfg->tnScale[2U] = 16u;

        for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
        {
            nsf4Cfg->tnCurve[cnt1][0u].posX  = 0;
            nsf4Cfg->tnCurve[cnt1][1u].posX  = 64;
            nsf4Cfg->tnCurve[cnt1][2u].posX  = 256;
            nsf4Cfg->tnCurve[cnt1][3u].posX  = 1024;
            nsf4Cfg->tnCurve[cnt1][4u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][5u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][6u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][7u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][8u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][9u].posX  = 4096;
            nsf4Cfg->tnCurve[cnt1][10u].posX = 4096;
            nsf4Cfg->tnCurve[cnt1][11u].posX = 4096;

            nsf4Cfg->tnCurve[cnt1][0u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][1u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][2u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][3u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][4u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][5u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][6u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][7u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][8u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][9u].posY  = 0;
            nsf4Cfg->tnCurve[cnt1][10u].posY = 0;
            nsf4Cfg->tnCurve[cnt1][11u].posY = 0;

            nsf4Cfg->tnCurve[cnt1][0u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][1u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][2u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][3u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][4u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][5u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][6u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][7u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][8u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][9u].slope  = 0;
            nsf4Cfg->tnCurve[cnt1][10u].slope = 0;
            nsf4Cfg->tnCurve[cnt1][11u].slope = 0;
        }

        lsccCfg->enable         = 0U;
        lsccCfg->lensCenterX    = 0U;
        lsccCfg->lensCenterY    = 0U;
        lsccCfg->tCfg           = 0U;
        lsccCfg->khCfg          = 0U;
        lsccCfg->kvCfg          = 0U;
        lsccCfg->gMaxCfg        = 0U;
        lsccCfg->setSel         = 0U;

        for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
        {
            for (cnt2 = 0U; cnt2 < NSF4_LSCC_MAX_SEGMENT; cnt2 ++)
            {
                 lsccCfg->pwlCurve[cnt1][cnt2].posX = 0;
                 lsccCfg->pwlCurve[cnt1][cnt2].posY = 0;
                 lsccCfg->pwlCurve[cnt1][cnt2].slope = 0;
             }
        }

        for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
        {
            nsf4Cfg->gains[cnt1] = 512u;
        }

        vissObj->vissCfgRef.nsf4Cfg = nsf4Cfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapCacParams(tivxVpacVissObj *vissObj)
{
#if defined(VPAC3) || defined(VPAC3L) 
    Cac_Config  *cacCfg;

    if (NULL != vissObj)
    {
        cacCfg = &vissObj->vissCfg.cacCfg;

        cacCfg->colorEnable = 6U;
        cacCfg->blkSize = 8U;
        cacCfg->blkGridSize.hCnt = 0u;
        cacCfg->blkGridSize.vCnt = 0u;
        cacCfg->displacementLut = cac_lut;

        vissObj->vissCfgRef.cacCfg = cacCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
#endif
}

static void tivxVpacVissDefaultMapPcidParams(tivxVpacVissObj *vissObj)
{
#if defined(VPAC3L)
    Pcid_Cfg    *pcidCfg;

    if (NULL != vissObj)
    {
        pcidCfg = &vissObj->vissCfg.pcidCfg;

        pcidCfg->cfaFormat = 1U;

        pcidCfg->opChCfg.irOutEn = FALSE;
        pcidCfg->opChCfg.bayerOutEn = TRUE;
        pcidCfg->opChCfg.bayerOutSel = PCID_BAYEROUTSEL_IR_SUB_BAYER;
        pcidCfg->opChCfg.rbIntpAtIR = PCID_COLOR_B_AT_IR_PIX_R_AT_B_PIX;
        pcidCfg->opChCfg.rbIRIntpMethod = PCID_COLOR_INTERPOLATE_HUE;
        pcidCfg->opChCfg.irSubtractEn = TRUE;

        pcidCfg->thRBIrCfg.t1 = 8192U;
        pcidCfg->thRBIrCfg.t2 = 16320U;
        pcidCfg->thRBIrCfg.t3 = 32768U;

        pcidCfg->clrDiffRBIrCfg.gHFXferFactorIr = 128U;
        pcidCfg->clrDiffRBIrCfg.gHFXferFactor = 128U;

        pcidCfg->irSubCfg.irSubtractFiltEn = TRUE;
        pcidCfg->irSubCfg.irRemapLutEn = FALSE;
        pcidCfg->irSubCfg.pIRRemapLut = NULL;

        pcidCfg->irSubCfg.cutOffTh = 64745U;
        pcidCfg->irSubCfg.transitionRange = 5U;
        pcidCfg->irSubCfg.transitionRangeInv = 51U;
        pcidCfg->irSubCfg.irSubDistScaleLut[0U] = 256U;
        pcidCfg->irSubCfg.irSubDistScaleLut[1U] = 256U;
        pcidCfg->irSubCfg.irSubDistScaleLut[2U] = 256U;
        pcidCfg->irSubCfg.irSubDistScaleLut[3U] = 256U;
        pcidCfg->irSubCfg.irSubDistScaleLut[4U] = 256U;
        pcidCfg->irSubCfg.irSubFactScale[0U] = 256U;
        pcidCfg->irSubCfg.irSubFactScale[1U] = 256U;
        pcidCfg->irSubCfg.irSubFactScale[2U] = 256U;
        pcidCfg->irSubCfg.irSubFactScale[3U] = 256U;

        vissObj->vissCfgRef.pcidCfg = pcidCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
#endif
}


static void tivxVpacVissDefaultMapHistParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_HistConfig  *histCfg;

    if (NULL != vissObj)
    {
        histCfg = &vissObj->vissCfg.fcpCfg[fcp_index].histCfg;

        histCfg->enable = (uint32_t) FALSE;
        histCfg->input = FCP_HIST_IN_SEL_COLOR_RED;
        histCfg->roi.cropStartX = 0u;
        histCfg->roi.cropStartY = 0u;
        histCfg->roi.cropWidth = 500u;
        histCfg->roi.cropHeight = 500u;

        vissObj->vissCfgRef.fcpCfg[fcp_index].histCfg = histCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapRGBLutParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_YuvSatLutConfig *yuvSatLutCfg;

    if (NULL != vissObj)
    {
        yuvSatLutCfg = &vissObj->vissCfg.fcpCfg[fcp_index].yuvSatLutCfg;

        yuvSatLutCfg->lumaInputBits = 10u;
        yuvSatLutCfg->enableLumaLut = (uint32_t) FALSE;
        yuvSatLutCfg->lumaLutAddr = gflexcc_lut_12to8;
        yuvSatLutCfg->enableChromaLut = (uint32_t) FALSE;
        yuvSatLutCfg->chromaLutAddr = gflexcc_lut_12to8;
        yuvSatLutCfg->enableSaturLut = (uint32_t) FALSE;
        yuvSatLutCfg->saturLutAddr = gflexcc_lut_12to8;

        vissObj->vissCfgRef.fcpCfg[fcp_index].yuvSatLutCfg = yuvSatLutCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapGammaParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_GammaConfig *gamma;

    if (NULL != vissObj)
    {
        gamma = &vissObj->vissCfg.fcpCfg[fcp_index].gammaCfg;

        gamma->enable = (uint32_t)TRUE;
        gamma->outClip = 10u;
        gamma->tableC1 = gflexcc_contrast_lut;
        gamma->tableC2 = gflexcc_contrast_lut;
        gamma->tableC3 = gflexcc_contrast_lut;

        vissObj->vissCfgRef.fcpCfg[fcp_index].gamma = gamma;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapRGB2HSVParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_Rgb2HsvConfig  *r2h = NULL;

    if (NULL != vissObj)
    {
        r2h = &vissObj->vissCfg.fcpCfg[fcp_index].rgb2hsvCfg;

        r2h->inputSelect = FCP_RGB2HSV_INPUT_CONTRAST_OUTPUT;
        r2h->h1Input = FCP_RGB2HSV_H1_INPUT_RED_COLOR;
        r2h->h2Input = FCP_RGB2HSV_H2_INPUT_BLUE_COLOR;
        r2h->weights[0u] = 64u;
        r2h->weights[1u] = 64u;
        r2h->weights[2u] = 128u;
        r2h->offset = 0u;
        r2h->useWbDataForGreyCalc = FALSE;
        r2h->wbOffset[0u] = 0u;
        r2h->wbOffset[1u] = 0u;
        r2h->wbOffset[2u] = 0u;
        r2h->threshold[0u] = 0u;
        r2h->threshold[1u] = 0u;
        r2h->threshold[2u] = 0u;
        r2h->satMinThr = 0u;
        r2h->satMode = FCP_SAT_MODE_SUM_RGB_MINUS_MIN_RGB;
        r2h->satDiv = FCP_SAT_DIV_4096_MINUS_GREY;

        vissObj->vissCfgRef.fcpCfg[fcp_index].rgb2Hsv = r2h;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapRGB2YUVParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_Rgb2YuvConfig  *r2y = NULL;

    if (NULL != vissObj)
    {
        r2y = &vissObj->vissCfg.fcpCfg[fcp_index].rgb2yuvCfg;

        r2y->weights[0u][0u] = 77;
        r2y->weights[0u][1u] = 150;
        r2y->weights[0u][2u] = 29;
        r2y->weights[1u][0u] = -44;
        r2y->weights[1u][1u] = -84;
        r2y->weights[1u][2u] = 128;
        r2y->weights[2u][0u] = 128;
        r2y->weights[2u][1u] = -108;
        r2y->weights[2u][2u] = -20;

        r2y->offsets[0u]     = 0;
        r2y->offsets[1u]     = 128;
        r2y->offsets[2u]     = 128;

        vissObj->vissCfgRef.fcpCfg[fcp_index].rgb2yuv = r2y;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index)
{
    uint32_t            cnt1, cnt2;
    Fcp_CcmConfig      *ccmCfg = NULL;

    if (NULL != vissObj)
    {
        ccmCfg = &vissObj->vissCfg.fcpCfg[fcp_index].ccmCfg;

        /* Map DCC Output Config to FVID2 Driver Config */
        for (cnt1 = 0u; cnt1 < FCP_MAX_CCM_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_CCM_COEFF_IN_RAW; cnt2 ++)
            {
                ccmCfg->weights[cnt1][cnt2] = 0;
            }
            ccmCfg->offsets[cnt1] = 0;
        }

        ccmCfg->weights[0][0] = 256;
        ccmCfg->weights[1][1] = 256;
        ccmCfg->weights[2][2] = 256;
        vissObj->vissCfgRef.fcpCfg[fcp_index].ccm = ccmCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    vissObj->vissCfgRef.lPwlCfg = &vissObj->vissCfg.pwlCfg1;
    vissObj->vissCfgRef.sPwlCfg = &vissObj->vissCfg.pwlCfg2;
    vissObj->vissCfgRef.vsPwlCfg = &vissObj->vissCfg.pwlCfg2;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

static void tivxVpacVissDefaultMapEeParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index)
{
    Fcp_EeConfig *eeCfg = NULL;

    if (NULL != vissObj)
    {
        eeCfg = &vissObj->vissCfg.fcpCfg[fcp_index].eeCfg;

        eeCfg->enable = TRUE;
        eeCfg->alignY12withChroma = FALSE;
        eeCfg->alignY8withChroma = FALSE;
        eeCfg->eeForY12OrY8 = 0;
        eeCfg->bypassY12 = FALSE;
        eeCfg->bypassC12 = TRUE;
        eeCfg->bypassY8 = TRUE;
        eeCfg->bypassC8 = TRUE;
        eeCfg->leftShift = 0;
        eeCfg->rightShift = 0;
        eeCfg->yeeShift = 4;
        eeCfg->coeff[0] = -1;
        eeCfg->coeff[1] = -3;
        eeCfg->coeff[2] = -5;
        eeCfg->coeff[3] = -3;
        eeCfg->coeff[4] = -2;
        eeCfg->coeff[5] =  2;
        eeCfg->coeff[6] = -5;
        eeCfg->coeff[7] =  2;
        eeCfg->coeff[8] = 48;
        eeCfg->yeeEThr = 0;
        eeCfg->yeeMergeSel = 0;
        eeCfg->haloReductionOn = 1;
        eeCfg->yesGGain = 0;
        eeCfg->yesEGain = 0;
        eeCfg->yesEThr1 = 0;
        eeCfg->yesEThr2 = 0;
        eeCfg->yesGOfset = 0;
        eeCfg->lut = yee_lut;

        vissObj->vissCfgRef.fcpCfg[fcp_index].eeCfg = eeCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapGlbceParams(tivxVpacVissObj *vissObj)
{
    Glbce_Config            *glbceCfg = NULL;
    Glbce_PerceptConfig     *prcptCfg = NULL;

    if (NULL != vissObj)
    {
        glbceCfg = &vissObj->vissCfg.glbceCfg;

        glbceCfg->irStrength = 255;
        glbceCfg->blackLevel = 0;
        glbceCfg->whiteLevel = 65535;
        glbceCfg->intensityVariance = 0xC;
        glbceCfg->spaceVariance = 7;
        glbceCfg->brightAmplLimit = 6;
        glbceCfg->darkAmplLimit = 6;
        glbceCfg->dither = GLBCE_NO_DITHER;
        glbceCfg->maxSlopeLimit = 72;
        glbceCfg->minSlopeLimit = 62;

        /* Copy default global assym table */
        memcpy(glbceCfg->asymLut, gGlbceAsymTbl, GLBCE_ASYMMETRY_LUT_SIZE * 4U);

        vissObj->vissCfgRef.glbceCfg = glbceCfg;

        /* Copy default fwd perception table */
        prcptCfg = &vissObj->vissCfg.fwdPrcpCfg;
        prcptCfg->enable = (uint32_t)FALSE;
        memcpy(prcptCfg->table, gGlbceFwdPrcptTbl, GLBCE_PERCEPT_LUT_SIZE * 4U);

        vissObj->vissCfgRef.fwdPrcpCfg = prcptCfg;

        /* Copy default rev perception table */
        prcptCfg = &vissObj->vissCfg.revPrcpCfg;
        prcptCfg->enable = (uint32_t)FALSE;
        memcpy(prcptCfg->table, gGlbceRevPrcptTbl, GLBCE_PERCEPT_LUT_SIZE * 4U);

        vissObj->vissCfgRef.revPrcpCfg = prcptCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }

}

static void tivxVpacVissDefaultMapFcpParams(tivxVpacVissObj *vissObj)
{
    uint32_t fcp_index;

    for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
    {
        tivxVpacVissDefaultMapFlexCFAParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapFlexCCParams(vissObj, fcp_index);
    }
}

static void tivxVpacVissDefaultMapFlexCFAParams(tivxVpacVissObj *vissObj, uint32_t fcp_index)
{
    uint32_t cnt;
    Fcp_CfaConfig *cfaCfg = NULL;

    if (NULL != vissObj)
    {
        cfaCfg = &vissObj->vissCfg.fcpCfg[fcp_index].cfaCfg;

#if defined(VPAC3) || defined(VPAC3L)
        {
            Fcp_comDecomLutConfig *decomLutCfg =  &vissObj->vissCfg.fcpCfg[fcp_index].decomLutCfg;
            Fcp_comDecomLutConfig *comLutCfg   =  &vissObj->vissCfg.fcpCfg[fcp_index].comLutCfg;

            if( NULL != decomLutCfg )
            {
                decomLutCfg->enable = 0;

                for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
                {
                    decomLutCfg->tableAddr[cnt] = NULL;
                }
                vissObj->vissCfgRef.fcpCfg[fcp_index].decomLutCfg = decomLutCfg;
            }

            if( NULL != comLutCfg )
            {
                comLutCfg->enable = 0;

                for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
                {
                    comLutCfg->tableAddr[cnt] = NULL;
                }
                vissObj->vissCfgRef.fcpCfg[fcp_index].comLutCfg = comLutCfg;
            }

            cfaCfg->enable16BitMode = 0;
            cfaCfg->linearBitWidth = 12U;
            cfaCfg->ccmEnable = 0;

            for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
            {
                cfaCfg->firConfig[cnt].enable = 0;
                cfaCfg->firConfig[cnt].scaler = 1U<<8U;
                cfaCfg->firConfig[cnt].offset = 0;

                cfaCfg->ccmConfig[cnt].inputCh0 = 0;
                cfaCfg->ccmConfig[cnt].inputCh1 = 0;
                cfaCfg->ccmConfig[cnt].inputCh2 = 0;
                cfaCfg->ccmConfig[cnt].inputCh3 = 0;
                cfaCfg->ccmConfig[cnt].offset = 0;
            }
        }
#endif

        /* DCC does not support CFA, so using default config from
         * example_Sensor/0/0/flexCFA_cfg test case */
        for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
        {
            cfaCfg->bypass[cnt] = (uint32_t)FALSE;
        }

        for (cnt = 0u; cnt < FCP_MAX_CFA_COEFF; cnt ++)
        {
            cfaCfg->coeff[cnt] = gcfa_coeff[cnt];
        }

        for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
        {
            cfaCfg->coreSel[cnt] = FCP_CFA_CORE_SEL_CORE0;
            cfaCfg->coreBlendMode[cnt] = FCP_CFA_CORE_BLEND_MODE_INPUT012;
        }

        for (cnt = 0u; cnt < FCP_CFA_MAX_SET; cnt ++)
        {
            if (0u == cnt)
            {
                cfaCfg->gradHzPh[cnt][0u] = 175u;
                cfaCfg->gradHzPh[cnt][1u] = 95u;
                cfaCfg->gradHzPh[cnt][2u] = 95u;
                cfaCfg->gradHzPh[cnt][3u] = 175u;
            }
            else
            {
                cfaCfg->gradHzPh[cnt][0u] = 175u;
                cfaCfg->gradHzPh[cnt][1u] = 195u;
                cfaCfg->gradHzPh[cnt][2u] = 195u;
                cfaCfg->gradHzPh[cnt][3u] = 175u;
            }

            if (0u == cnt)
            {
                cfaCfg->gradVtPh[cnt][0u] = 175u;
                cfaCfg->gradVtPh[cnt][1u] = 95u;
                cfaCfg->gradVtPh[cnt][2u] = 95u;
                cfaCfg->gradVtPh[cnt][3u] = 175u;
            }
            else
            {
                cfaCfg->gradVtPh[cnt][0u] = 276u;
                cfaCfg->gradVtPh[cnt][1u] = 196u;
                cfaCfg->gradVtPh[cnt][2u] = 196u;
                cfaCfg->gradVtPh[cnt][3u] = 276u;
            }

            if (0u == cnt)
            {
                cfaCfg->intsBitField[cnt][0U] = 0u;
                cfaCfg->intsBitField[cnt][1U] = 1u;
                cfaCfg->intsBitField[cnt][2U] = 2u;
                cfaCfg->intsBitField[cnt][3U] = 3u;
            }
            else
            {
                cfaCfg->intsBitField[cnt][0U] = 8u;
                cfaCfg->intsBitField[cnt][1U] = 9u;
                cfaCfg->intsBitField[cnt][2U] = 10u;
                cfaCfg->intsBitField[cnt][3U] = 11u;
            }

            if (0u == cnt)
            {
                cfaCfg->intsShiftPh[cnt][0u] = 4u;
                cfaCfg->intsShiftPh[cnt][1u] = 5u;
                cfaCfg->intsShiftPh[cnt][2u] = 6u;
                cfaCfg->intsShiftPh[cnt][3u] = 7u;
            }
            else
            {
                cfaCfg->intsShiftPh[cnt][0u] = 12u;
                cfaCfg->intsShiftPh[cnt][1u] = 13u;
                cfaCfg->intsShiftPh[cnt][2u] = 14u;
                cfaCfg->intsShiftPh[cnt][3u] = 15u;
            }

            if (0u == cnt)
            {
                cfaCfg->thr[cnt][0u] = 500u;
                cfaCfg->thr[cnt][1u] = 600u;
                cfaCfg->thr[cnt][2u] = 700u;
                cfaCfg->thr[cnt][3u] = 800u;
                cfaCfg->thr[cnt][4u] = 900u;
                cfaCfg->thr[cnt][5u] = 1000u;
                cfaCfg->thr[cnt][6u] = 1100u;
            }
            else
            {
                cfaCfg->thr[cnt][0u] = 0u;
                cfaCfg->thr[cnt][1u] = 100u;
                cfaCfg->thr[cnt][2u] = 200u;
                cfaCfg->thr[cnt][3u] = 300u;
                cfaCfg->thr[cnt][4u] = 400u;
                cfaCfg->thr[cnt][5u] = 500u;
                cfaCfg->thr[cnt][6u] = 600u;
            }
        }

        {
            Vhwa_LutConfig *lut16to12Cfg = &vissObj->vissCfg.fcpCfg[fcp_index].cfaLut16to12Cfg;
            lut16to12Cfg->enable    = 0u;
            lut16to12Cfg->inputBits = 12u;
            lut16to12Cfg->tableAddr = gcfa_lut_16to12;
            vissObj->vissCfgRef.fcpCfg[fcp_index].cfaLut16to12Cfg = lut16to12Cfg;
        }

        vissObj->vissCfgRef.fcpCfg[fcp_index].cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapFlexCCParams(tivxVpacVissObj *vissObj, uint32_t fcp_index)
{
    if (NULL != vissObj)
    {
        tivxVpacVissDefaultMapCCMParams(vissObj, NULL, fcp_index);
        tivxVpacVissDefaultMapRGB2YUVParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapRGB2HSVParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapGammaParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapRGBLutParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapHistParams(vissObj, fcp_index);
        tivxVpacVissDefaultMapEeParams(vissObj, fcp_index);
    }
}

static void tivxVpacVissDefaultMapMergeParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id)
{
    uint32_t cnt;
    Rfe_WdrConfig *mergeCfg;

    if (NULL != vissObj)
    {
        if (0U == inst_id)
        {
            mergeCfg = &vissObj->vissCfg.merge1Cfg;

            memset (mergeCfg, 0x0, sizeof(Rfe_WdrConfig));

            mergeCfg->enable                    = (uint32_t)FALSE;
            mergeCfg->dst                       = 15u;
            mergeCfg->sbit                      = 4u;
            mergeCfg->lbit                      = 4u;
            mergeCfg->gshort                    = 32768u;
            mergeCfg->glong                     = 2048u;
            for (cnt = 0u; cnt < RFE_MAX_COLOR_COMP; cnt ++)
            {
                mergeCfg->lwb[cnt]              = 512u;
                mergeCfg->swb[cnt]              = 512u;
            }
            mergeCfg->wdrThr                    = 4094u;
            mergeCfg->mad                       = 65535u;
            mergeCfg->mergeClip                 = 262143u;

            vissObj->vissCfgRef.wdr1Cfg         = mergeCfg;
        }
        else if (1U == inst_id)
        {
            mergeCfg = &vissObj->vissCfg.merge2Cfg;

            memset (mergeCfg, 0x0, sizeof(Rfe_WdrConfig));

            mergeCfg->enable                    = (uint32_t)FALSE;
            mergeCfg->dst                       = 15u;
            mergeCfg->sbit                      = 8u;
            mergeCfg->lbit                      = 8u;
            mergeCfg->gshort                    = 32768u;
            mergeCfg->glong                     = 128u;
            for (cnt = 0u; cnt < RFE_MAX_COLOR_COMP; cnt ++)
            {
                mergeCfg->lwb[cnt]              = 512u;
                mergeCfg->swb[cnt]              = 512u;
            }
            mergeCfg->wdrThr                    = 65504u;
            mergeCfg->mad                       = 65535u;
            mergeCfg->mergeClip                 = 262143u;

            vissObj->vissCfgRef.wdr2Cfg         = mergeCfg;
        }
        else
        {
            /* do nothing */
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapRfeLutParams(tivxVpacVissObj *vissObj, uint32_t lut_id)
{
    Vhwa_LutConfig              *lutCfg;

    if (NULL != vissObj)
    {
        /* lut for 20 to 16 bit conversion */
        if (0u == lut_id)
        {
            lutCfg = &vissObj->vissCfg.rfeLut20to16Cfg;

            lutCfg->enable    = 0u;
            lutCfg->inputBits = 16u;
            lutCfg->clip      = 4095u;
            lutCfg->tableAddr = grawfe_lut_20to16;

            vissObj->vissCfgRef.comp20To16LutCfg = lutCfg;
        }
        else if (1u == lut_id) /* H3A Lut */
        {
            lutCfg = &vissObj->vissCfg.h3aLutCfg;

            lutCfg->enable = 0u;
            vissObj->vissCfgRef.h3aLutCfg = NULL;
        }
        else
        {
            /* do nothing */
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapDpcOtfParams(tivxVpacVissObj *vissObj)
{
    Rfe_DpcOtfConfig *dpcOtfCfg;

    if (NULL != vissObj)
    {
        dpcOtfCfg = &vissObj->vissCfg.dpcOtfCfg;

#if defined (VPAC3L)
        dpcOtfCfg->enable           = (uint32_t)FALSE;

        dpcOtfCfg->threshold[0u]    = 100u;
        dpcOtfCfg->threshold[1u]    = 100u;
        dpcOtfCfg->threshold[2u]    = 200u;
        dpcOtfCfg->threshold[3u]    = 300u;
        dpcOtfCfg->threshold[4u]    = 500u;
        dpcOtfCfg->threshold[5u]    = 800u;
        dpcOtfCfg->threshold[6u]    = 1600u;
        dpcOtfCfg->threshold[7u]    = 3200u;

        dpcOtfCfg->slope[0u]        = 0u;
        dpcOtfCfg->slope[1u]        = 58u;
        dpcOtfCfg->slope[2u]        = 51u;
        dpcOtfCfg->slope[3u]        = 51u;
        dpcOtfCfg->slope[4u]        = 38u;
        dpcOtfCfg->slope[5u]        = 51u;
        dpcOtfCfg->slope[6u]        = 51u;
        dpcOtfCfg->slope[7u]        = 0u;

        dpcOtfCfg->cfa_mode         = RFE_CFA_CFG_MODE_4;
        dpcOtfCfg->cfa_phase        = 2;

        dpcOtfCfg->thrx[0u]         = 0u;
        dpcOtfCfg->thrx[1u]         = 60u;
        dpcOtfCfg->thrx[2u]         = 500u;
        dpcOtfCfg->thrx[3u]         = 1000u;
        dpcOtfCfg->thrx[4u]         = 2000u;
        dpcOtfCfg->thrx[5u]         = 4000u;
        dpcOtfCfg->thrx[6u]         = 8000u;
        dpcOtfCfg->thrx[7u]         = 1600u;

        dpcOtfCfg->lut2_threshold[0u]   = 50u;
        dpcOtfCfg->lut2_threshold[1u]   = 50u;
        dpcOtfCfg->lut2_threshold[2u]   = 100u;
        dpcOtfCfg->lut2_threshold[3u]   = 150u;
        dpcOtfCfg->lut2_threshold[4u]   = 250u;
        dpcOtfCfg->lut2_threshold[5u]   = 400u;
        dpcOtfCfg->lut2_threshold[6u]   = 800u;
        dpcOtfCfg->lut2_threshold[7u]   = 1600u;  

        dpcOtfCfg->lut2_slope[0u]    = 0u;
        dpcOtfCfg->lut2_slope[1u]    = 29u;
        dpcOtfCfg->lut2_slope[2u]    = 26u;
        dpcOtfCfg->lut2_slope[3u]    = 26u;
        dpcOtfCfg->lut2_slope[4u]    = 19u;
        dpcOtfCfg->lut2_slope[5u]    = 26u;
        dpcOtfCfg->lut2_slope[6u]    = 26u;
        dpcOtfCfg->lut2_slope[7u]    = 0u;  

        dpcOtfCfg->lut2_thrx[0u]    = 0u;
        dpcOtfCfg->lut2_thrx[1u]    = 60u;
        dpcOtfCfg->lut2_thrx[2u]    = 500u;
        dpcOtfCfg->lut2_thrx[3u]    = 1000u;
        dpcOtfCfg->lut2_thrx[4u]    = 2000u;
        dpcOtfCfg->lut2_thrx[5u]    = 4000u;
        dpcOtfCfg->lut2_thrx[6u]    = 8000u;
        dpcOtfCfg->lut2_thrx[7u]    = 16000u;        

        dpcOtfCfg->lut_map          = 8u; 
#else
        dpcOtfCfg->enable           = (uint32_t)FALSE;
        dpcOtfCfg->threshold[0u]    = 200u;
        dpcOtfCfg->slope[0u]        = 0u;
        dpcOtfCfg->threshold[1u]    = 200u;
        dpcOtfCfg->slope[1u]        = 50u;
        dpcOtfCfg->threshold[2u]    = 300u;
        dpcOtfCfg->slope[2u]        = 50u;
        dpcOtfCfg->threshold[3u]    = 500u;
        dpcOtfCfg->slope[3u]        = 37u;
        dpcOtfCfg->threshold[4u]    = 800u;
        dpcOtfCfg->slope[4u]        = 50u;
        dpcOtfCfg->threshold[5u]    = 1600u;
        dpcOtfCfg->slope[5u]        = 50u;
        dpcOtfCfg->threshold[6u]    = 3200u;
        dpcOtfCfg->slope[6u]        = 50u;
        dpcOtfCfg->threshold[7u]    = 6400u;
        dpcOtfCfg->slope[7u]        = 50u;
#endif

        vissObj->vissCfgRef.dpcOtf  = dpcOtfCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapDpcLutParams(tivxVpacVissObj *vissObj)
{
    Rfe_DpcLutConfig *dpcLutCfg;

    if (NULL != vissObj)
    {
        dpcLutCfg = &vissObj->vissCfg.dpcLutCfg;

        dpcLutCfg->enable           = (uint32_t)FALSE;
        dpcLutCfg->isReplaceWhite   = (uint32_t)FALSE;
        dpcLutCfg->maxDefectPixels  = 0x1u;

        vissObj->vissCfgRef.dpcLut  = dpcLutCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapLscParams(tivxVpacVissObj *vissObj)
{
    Rfe_LscConfig *lscCfg;

    if (NULL != vissObj)
    {
        lscCfg = &vissObj->vissCfg.lscCfg;

#if defined (VPAC3L)
        lscCfg->enable          = (uint32_t)FALSE;
        lscCfg->gainFmt         = RFE_LSC_GAIN_FMT_U8Q7_1;
        lscCfg->horzDsFactor    = RFE_LSC_DS_FACTOR_64;
        lscCfg->vertDsFactor    = RFE_LSC_DS_FACTOR_64;
        lscCfg->tableAddr       = grawfe_lsc_tbl;
        lscCfg->numTblEntry     = 0x0u;
        lscCfg->chn_mode        = RFE_LSC_CHNMODE_8;
        lscCfg->lut_map[0]      = lscCfg->lut_map[8]    = 0;  
        lscCfg->lut_map[1]      = lscCfg->lut_map[9]    = 1;  
        lscCfg->lut_map[2]      = lscCfg->lut_map[10]   = 2;  
        lscCfg->lut_map[3]      = lscCfg->lut_map[11]   = 3;   
        lscCfg->lut_map[4]      = lscCfg->lut_map[12]   = 4;  
        lscCfg->lut_map[5]      = lscCfg->lut_map[13]   = 5;  
        lscCfg->lut_map[6]      = lscCfg->lut_map[14]   = 6;  
        lscCfg->lut_map[7]      = lscCfg->lut_map[15]   = 7;  
#else
        lscCfg->enable          = (uint32_t)FALSE;
        lscCfg->gainFmt         = RFE_LSC_GAIN_FMT_U8Q8;
        lscCfg->horzDsFactor    = RFE_LSC_DS_FACTOR_32;
        lscCfg->vertDsFactor    = RFE_LSC_DS_FACTOR_16;
        lscCfg->tableAddr       = grawfe_lsc_tbl;
        lscCfg->numTblEntry     = 0x0u;
#endif

        vissObj->vissCfgRef.lscCfg = lscCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapWb2Params(tivxVpacVissObj *vissObj)
{
    Rfe_GainOfstConfig *wbCfg;

    if (NULL != vissObj)
    {
        wbCfg = &vissObj->vissCfg.wbCfg;

        wbCfg->gain[0u] = 512u;
        wbCfg->gain[1u] = 512u;
        wbCfg->gain[2u] = 512u;
        wbCfg->gain[3u] = 512u;

        wbCfg->offset[0u] = 0u;
        wbCfg->offset[1u] = 0u;
        wbCfg->offset[2u] = 0u;
        wbCfg->offset[3u] = 0u;

        vissObj->vissCfgRef.wbCfg = wbCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id)
{
    Rfe_PwlConfig   *pwlCfg;
    Vhwa_LutConfig  *lutCfg;

    if (NULL != vissObj)
    {
        if (0u == inst_id) /* Long Exposure */
        {
            pwlCfg = &vissObj->vissCfg.pwlCfg1;
            lutCfg = &vissObj->vissCfg.decomp1Cfg;

            pwlCfg->mask         = 4095u;
            pwlCfg->shift        = 0u;
            pwlCfg->offset[0u]   = 0;
            pwlCfg->offset[1u]   = 0;
            pwlCfg->offset[2u]   = 0;
            pwlCfg->offset[3u]   = 0;
            pwlCfg->gain[0u]     = 512u;
            pwlCfg->gain[1u]     = 512u;
            pwlCfg->gain[2u]     = 512u;
            pwlCfg->gain[3u]     = 512u;

            pwlCfg->enable       = 0u;
            pwlCfg->xthr1        = 128u;
            pwlCfg->xthr2        = 256u;
            pwlCfg->xthr3        = 512u;
            pwlCfg->ythr1        = 2048u;
            pwlCfg->ythr2        = 4096u;
            pwlCfg->ythr3        = 8192u;
            pwlCfg->slope1       = 16u;
            pwlCfg->slope2       = 16u;
            pwlCfg->slope3       = 16u;
            pwlCfg->slope4       = 16u;
            pwlCfg->slopeShift   = 0u;
            pwlCfg->outClip      = 65535;

            lutCfg->enable       = 0u;
            lutCfg->inputBits    = 20u;
            lutCfg->clip         = 65535;
            lutCfg->tableAddr    = grawfe_pwl_long_lut;

            vissObj->vissCfgRef.lPwlCfg = pwlCfg;
            vissObj->vissCfgRef.lLutCfg = lutCfg;
        }
        else if (1u == inst_id)
        {
            pwlCfg = &vissObj->vissCfg.pwlCfg2;
            lutCfg = &vissObj->vissCfg.decomp2Cfg;

            pwlCfg->mask        = 4095u;
            pwlCfg->shift       = 0u;     /* 3 bits  */
            pwlCfg->offset[0u]  = 0;   //S16
            pwlCfg->offset[1u]  = 0;   //S16
            pwlCfg->offset[2u]  = 0;   //S16
            pwlCfg->offset[3u]  = 0;   //S16
            pwlCfg->gain[0u]    = 512u;     //U13Q9
            pwlCfg->gain[1u]    = 512u;     //U13Q9
            pwlCfg->gain[2u]    = 512u;     //U13Q9
            pwlCfg->gain[3u]    = 512u;     //U13Q9

            pwlCfg->enable      = 0u;
            pwlCfg->xthr1       = 128u;       /* 16 bits */
            pwlCfg->xthr2       = 128u;
            pwlCfg->xthr3       = 512u;
            pwlCfg->ythr1       = 2048u;       /* 24 bits */
            pwlCfg->ythr2       = 4096u;
            pwlCfg->ythr3       = 8192u;
            pwlCfg->slope1      = 16u;      /* 16 bits */
            pwlCfg->slope2      = 16u;
            pwlCfg->slope3      = 16u;
            pwlCfg->slope4      = 16u;
            pwlCfg->slopeShift  = 0u;  /* Shift for Q point of slope */
            pwlCfg->outClip     = 65535;     /* 24 bits */

            lutCfg->enable       = 0u;
            lutCfg->inputBits    = 20u;
            lutCfg->clip         = 65535;
            lutCfg->tableAddr    = grawfe_pwl_short_lut;

            vissObj->vissCfgRef.sPwlCfg = pwlCfg;
            vissObj->vissCfgRef.sLutCfg = lutCfg;
        }
        else if (2u == inst_id)
        {
            pwlCfg = &vissObj->vissCfg.pwlCfg3;
            lutCfg = &vissObj->vissCfg.decomp3Cfg;

            pwlCfg->mask        = 4095u;
            pwlCfg->shift       = 0u;     /* 3 bits  */
            pwlCfg->offset[0u]  = 0;
            pwlCfg->offset[1u]  = 0;
            pwlCfg->offset[2u]  = 0;
            pwlCfg->offset[3u]  = 0;
            pwlCfg->gain[0u]    = 512u;     //U13Q9
            pwlCfg->gain[1u]    = 512u;     //U13Q9
            pwlCfg->gain[2u]    = 512u;     //U13Q9
            pwlCfg->gain[3u]    = 512u;     //U13Q9

            pwlCfg->enable      = 0u;
            pwlCfg->xthr1       = 512u;       /* 16 bits */
            pwlCfg->xthr2       = 1408u;
            pwlCfg->xthr3       = 2176u;
            pwlCfg->ythr1       = 2048u;       /* 24 bits */
            pwlCfg->ythr2       = 16384u;
            pwlCfg->ythr3       = 65536u;
            pwlCfg->slope1      = 4u;      /* 16 bits */
            pwlCfg->slope2      = 16u;
            pwlCfg->slope3      = 64u;
            pwlCfg->slope4      = 512u;
            pwlCfg->slopeShift  = 0u;  /* Shift for Q point of slope */
            pwlCfg->outClip     = 1048575;     /* 24 bits */

            lutCfg->enable      = 0u;
            lutCfg->inputBits   = 20u;
            lutCfg->clip        = 65535;
            lutCfg->tableAddr   = grawfe_pwl_vshort_lut;

            vissObj->vissCfgRef.vsPwlCfg = pwlCfg;
            vissObj->vissCfgRef.vsLutCfg = lutCfg;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "inst_id (%d) is invalid, should be in range (0-2)\n", inst_id);
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDefaultMapRfeParams(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissDefaultMapPwlParams(vissObj, 0u);
        /* Parse Short exposure PWL Parameters */
        tivxVpacVissDefaultMapPwlParams(vissObj, 1u);
        /* Parse Very Short exposure PWL Parameters */
        tivxVpacVissDefaultMapPwlParams(vissObj, 2u);

        tivxVpacVissDefaultMapMergeParams(vissObj, 0u);
        tivxVpacVissDefaultMapMergeParams(vissObj, 1u);

        /* Parse Lut for 20 to 16 conversion */
        tivxVpacVissDefaultMapRfeLutParams(vissObj, 0u);

        /* Parse DPC OTF Parameters */
        tivxVpacVissDefaultMapDpcOtfParams(vissObj);
        /* Parse DPC OTF Parameters */
        tivxVpacVissDefaultMapDpcLutParams(vissObj);
        /* Parse LSC Parameters */
        tivxVpacVissDefaultMapLscParams(vissObj);
        /* Parse WB2 Parameters */
        tivxVpacVissDefaultMapWb2Params(vissObj);

        /* Parse H3A Source Parameters */
        tivxVpacVissDefaultMapRfeLutParams(vissObj, 1u);
    }
}

