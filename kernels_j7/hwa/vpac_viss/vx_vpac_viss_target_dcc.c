/*
 *
 * Copyright (c) 2019-2020 Texas Instruments Incorporated
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

static void tivxVpacVissDccMapH3aParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapNsf4Params(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapRGB2YUVParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapRGB2HSVParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapGammaParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapRGBLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapHistParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapDpcLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapDpcOtfParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapMergeParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);
static void tivxVpacVissDccMapLscParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapRfeLutParams(tivxVpacVissObj *vissObj,
    uint32_t lut_id);
static void tivxVpacVissDccMapWb2Params(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Creating LUT array here, so that only one copy is allocated for all handles
 * Once the DCC supports direct write to Driver structures, this should
 * be removed
 */
uint32_t gH3aLut[RFE_H3A_COMP_LUT_SIZE] = {0U};

uint32_t gcfa_lut_16to12[] =
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

uint32_t grawfe_lut_20to16[] =
{
    #include "rawfe_lut_20to16_0.txt"
};

uint32_t grawfe_lsc_tbl[] =
{
    #include "rawfe_lsc_tbl_0.txt"
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

uint32_t gGlbceAsymTbl[] =
{
    0,12173,20997,27687,32934,37159,40634,43543,46014,48138,49984,51603,53035,54310,55453,56483,57416,58265,59041,59753,60409,61015,61577,62099,62585,63039,63464,63863,64237,64590,64923,65237,65535,
};

uint32_t gGlbceFwdPrcptTbl[] =
{
    #include "glbce_fwd_percept_lut.txt"
};

uint32_t gGlbceRevPrcptTbl[] =
{
    #include "glbce_rev_percept_lut.txt"
};

int32_t yee_lut[] =
{
    #include "yee_lut.txt"
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissInitDcc(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vissObj->bypass_nsf4 = vissPrms->bypass_nsf4;

    vissObj->dcc_in_prms.analog_gain = 1000;
    vissObj->dcc_in_prms.cameraId = vissPrms->sensor_dcc_id;
    vissObj->dcc_in_prms.color_temparature = 5000;
    vissObj->dcc_in_prms.exposure_time = 33333;

    /* Calculate the number of bytes required for
     * storing DCC output */
    vissObj->dcc_out_numbytes = calc_dcc_outbuf_size();

    /* Allocate DCC output buffer */
    vissObj->dcc_out_buf = (uint8_t *)tivxMemAlloc(
        vissObj->dcc_out_numbytes, (vx_enum)TIVX_MEM_EXTERNAL);

    if (NULL == vissObj->dcc_out_buf)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissInitDcc: dcc_update Failed !!!\n");
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

void tivxVpacVissDeInitDcc(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj->dcc_out_buf)
    {
        tivxMemFree(vissObj->dcc_out_buf, vissObj->dcc_out_numbytes,
            (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

vx_status tivxVpacVissSetParamsFromDcc(tivxVpacVissObj *vissObj,
    const tivx_obj_desc_user_data_object_t *dcc_buf_desc,
    const tivx_obj_desc_user_data_object_t *h3a_out_desc,
    tivx_ae_awb_params_t *ae_awb_res)
{
    vx_status                   status = (vx_status)VX_SUCCESS;
    int32_t                         dcc_status;
    dcc_parser_input_params_t  *dcc_in_prms;
    dcc_parser_output_params_t *dcc_out_prms;

    if (NULL != dcc_buf_desc)
    {
        dcc_in_prms = &vissObj->dcc_in_prms;
        dcc_out_prms = &vissObj->dcc_out_prms;

        dcc_in_prms->dcc_buf_size = dcc_buf_desc->mem_size;
        dcc_in_prms->dcc_buf = tivxMemShared2TargetPtr(&dcc_buf_desc->mem_ptr);

        if(NULL != dcc_in_prms->dcc_buf)
        {
            tivxMemBufferMap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            dcc_status = Dcc_Create(dcc_out_prms, vissObj->dcc_out_buf);
            dcc_out_prms->useAwbCalbCfg = 0U;
            dcc_out_prms->useH3aCfg = 0U;
            dcc_out_prms->useNsf4Cfg = 0U;
            dcc_out_prms->useBlcCfg = 0U;
            dcc_out_prms->useCfaCfg = 0U;
            dcc_out_prms->useCcmCfg = 0U;
            dcc_out_prms->useH3aMuxCfg = 0U;
            dcc_out_prms->useRfeDcmpCfg = 0U;
            if (0 == dcc_status)
            {
                dcc_status = dcc_update(dcc_in_prms, dcc_out_prms);

                if (0 != dcc_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissSetParamsFromDcc: dcc_update Failed !!!\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissSetParamsFromDcc: Dcc_Create Failed !!!\n");
                status = (vx_status)VX_FAILURE;
            }

            tivxMemBufferUnmap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissSetParamsFromDcc: Incorrect DCC Input Buf memory !!!\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxVpacVissDccMapRfeParams(vissObj);
            tivxVpacVissDccMapNsf4Params(vissObj, ae_awb_res);
            if (NULL != h3a_out_desc)
            {
                tivxVpacVissDccMapH3aParams(vissObj, ae_awb_res);
            }
            tivxVpacVissDccMapH3aLutParams(vissObj);
            tivxVpacVissDccMapGlbceParams(vissObj);
            tivxVpacVissDccMapFlexCFAParams(vissObj);
            tivxVpacVissDccMapFlexCCParams(vissObj, ae_awb_res);

            tivxVpacVissDccMapBlc(vissObj, ae_awb_res);
        }
    }

    return (status);
}

void tivxVpacVissDccMapH3aLutParams(tivxVpacVissObj *vissObj)
{
    uint32_t            cnt;
    Vhwa_LutConfig     *h3aLutCfg = NULL;
    iss_h3a_mux_luts   *dccH3aLutCfg = NULL;

    h3aLutCfg = &vissObj->vissCfg.h3aLutCfg;
    dccH3aLutCfg = &vissObj->dcc_out_prms.issH3aMuxLuts;

    if (1U == dccH3aLutCfg->enable)
    {
        h3aLutCfg->enable = (uint32_t)TRUE;
        h3aLutCfg->inputBits = 16U;
        h3aLutCfg->clip = 0x3FFU;

        h3aLutCfg->tableAddr = &gH3aLut[0U];

        /* TODO: Select the h3a_mux_lut based on frame count. */
        for (cnt = 0U; cnt < RFE_H3A_COMP_LUT_SIZE; cnt ++)
        {
            gH3aLut[cnt] = dccH3aLutCfg->h3a_mux_lut[0U][cnt];
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }

    vissObj->vissCfgRef.h3aLutCfg = h3aLutCfg;
}

void tivxVpacVissSetH3aSrcParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    Rfe_H3aInConfig     *inCfg;

    if (((vx_bool)vx_true_e == vissObj->h3a_out_enabled) &&
        (vissPrms->h3a_in != vissObj->lastH3aInSrc))
    {
        inCfg = &vissObj->vissCfg.h3aInCfg;

        inCfg->inSel = vissPrms->h3a_in;

        /* Fixing downshift factor to 0. Mapping to 10-bit expected to come from DCC H3A LUT for linear as well as WDR dataflow */
        inCfg->shift = 0U;

        vissObj->vissCfgRef.rfeH3aInCfg = inCfg;

        /* Save locally H3A input Source */
        vissObj->lastH3aInSrc = vissPrms->h3a_in;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

vx_status tivxVpacVissApplyAEWBParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *aewb_result)
{
    vx_status                        status = (vx_status)VX_SUCCESS;
    int32_t                          dcc_status = 0;
    tivxVpacVissConfig              *vsCfg;
    Rfe_GainOfstConfig              *wbCfg;
    dcc_parser_input_params_t       *dcc_in_prms;
    dcc_parser_output_params_t      *dcc_out_prms;

    vsCfg = &vissObj->vissCfg;
    dcc_in_prms = &vissObj->dcc_in_prms;
    dcc_out_prms = &vissObj->dcc_out_prms;

    wbCfg = &vsCfg->wbCfg;

    /* apply AWB gains in RAWFE when NSF4 is bypassed */
    if ((1u == vissObj->bypass_nsf4) && (1u == aewb_result->awb_valid))
    {
        wbCfg->gain[0U] = aewb_result->wb_gains[0U];
        wbCfg->gain[1U] = aewb_result->wb_gains[1U];
        wbCfg->gain[2U] = aewb_result->wb_gains[2U];
        wbCfg->gain[3U] = aewb_result->wb_gains[3U];
    }

    /* Set even if wbCfg is not updated so that driver updates in
     * case of multi-instance */
    vissObj->vissCfgRef.wbCfg = wbCfg;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;

    if((1u == vissObj->use_dcc) &&
       ((1u == aewb_result->awb_valid) || (1u == aewb_result->ae_valid)))
    {
        if (1u == aewb_result->awb_valid)
        {
            dcc_in_prms->color_temparature = aewb_result->color_temperature;
        }

        if (1u == aewb_result->ae_valid)
        {
            dcc_in_prms->analog_gain = aewb_result->analog_gain;
            dcc_in_prms->exposure_time = aewb_result->exposure_time;
        }

        dcc_status = dcc_update(dcc_in_prms, dcc_out_prms);

        if (0 != dcc_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissApplyAEWBParams: DCC Update Failed !!!\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* Apply DCC Output to VISS Driver config */

            /* Apply DCC Output and update CCM */
            tivxVpacVissDccMapCCMParams(vissObj, aewb_result);

            /* Update WB Gains in NSF4 if it is enabled */
            if (0u == vissObj->bypass_nsf4)
            {
                tivxVpacVissDccMapNsf4Params(vissObj, aewb_result);
            }

            /* Update BLC Offset */
            tivxVpacVissDccMapBlc(vissObj, aewb_result);

            /*
             * TODO: H3A LUT Update.
             */
        }
    }
    else
    {
        /* In case of multi-instance, these need to be updated in
         * driver (required until there is better ctx restore mechanism)*/
        vissObj->vissCfgRef.ccm = &vissObj->vissCfg.ccmCfg;
        if (0u == vissObj->bypass_nsf4)
        {
            vissObj->vissCfgRef.nsf4Cfg = &vissObj->vissCfg.nsf4Cfg;
        }
        vissObj->vissCfgRef.lPwlCfg = &vissObj->vissCfg.pwlCfg1;
        vissObj->vissCfgRef.sPwlCfg = &vissObj->vissCfg.pwlCfg2;
        vissObj->vissCfgRef.vsPwlCfg = &vissObj->vissCfg.pwlCfg3;
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static void tivxVpacVissDccMapH3aParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    H3a_Config         *h3aCfg = NULL;
    iss_ipipe_h3a_aewb *dccH3aCfg = NULL;
    H3a_AewbConfig     *aewbCfg = NULL;

    h3aCfg = &vissObj->vissCfg.h3aCfg;
    aewbCfg = &h3aCfg->aewbCfg;
    dccH3aCfg = &vissObj->dcc_out_prms.ipipeH3A_AEWBCfg;

    /* MAP DCC config to FVID2 driver config */
    h3aCfg->module                  = H3A_MODULE_AEWB;
    h3aCfg->pos.startX              = 0u;
    h3aCfg->pos.startY              = 0u;
    aewbCfg->enableALowComp         = dccH3aCfg->ALaw_En;
    aewbCfg->enableMedFilt          = dccH3aCfg->MedFilt_En;
    /* TODO: h3aCfg->midFiltThreshold = */
    aewbCfg->winCfg.pos.startX      = dccH3aCfg->h_start;
    aewbCfg->winCfg.pos.startY      = dccH3aCfg->v_start;
    aewbCfg->winCfg.width           = dccH3aCfg->h_size;
    aewbCfg->winCfg.height          = dccH3aCfg->v_size;
    aewbCfg->winCfg.horzCount       = dccH3aCfg->h_count;
    aewbCfg->winCfg.vertCount       = dccH3aCfg->v_count;
    aewbCfg->winCfg.horzIncr        = dccH3aCfg->h_skip;
    aewbCfg->winCfg.vertIncr        = dccH3aCfg->v_skip;
    aewbCfg->blackLineVertStart     = dccH3aCfg->blk_row_vpos;
    aewbCfg->blackLineHeight        = dccH3aCfg->blk_win_numlines;
    aewbCfg->outMode                = dccH3aCfg->mode;
    aewbCfg->sumShift               = dccH3aCfg->sum_shift;
    aewbCfg->satLimit               = dccH3aCfg->saturation_limit;

    vissObj->vissCfgRef.h3aCfg = h3aCfg;

    vissObj->aew_config.aewwin1_WINH = (uint16_t)aewbCfg->winCfg.height;
    vissObj->aew_config.aewwin1_WINW = (uint16_t)aewbCfg->winCfg.width;
    vissObj->aew_config.aewwin1_WINVC = (uint16_t)aewbCfg->winCfg.vertCount;
    vissObj->aew_config.aewwin1_WINHC = (uint16_t)aewbCfg->winCfg.horzCount;
    vissObj->aew_config.aewsubwin_AEWINCV = (uint16_t)aewbCfg->winCfg.vertIncr;
    vissObj->aew_config.aewsubwin_AEWINCH = (uint16_t)aewbCfg->winCfg.horzIncr;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

static void tivxVpacVissDccMapNsf4Params(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t            cnt1, cnt2;
    Nsf4v_Config       *nsf4Cfg = NULL;
    Nsf4_LsccConfig    *lsccCfg = NULL;
    viss_nsf4          *dccNsf4Cfg = NULL;
    uint8_t            use_defaults = 1u;

    nsf4Cfg = &vissObj->vissCfg.nsf4Cfg;
    lsccCfg = &nsf4Cfg->lsccCfg;

    dccNsf4Cfg = &(vissObj->dcc_out_prms.vissNSF4Cfg);
    if(1U == vissObj->dcc_out_prms.useNsf4Cfg)
    {
        use_defaults = 0;
    }

    /* Map DCC Output Config to FVID2 Driver Config */

    /* TODO: Does shading gain map to lscc enable? */

    if(0u == use_defaults)
    {
        nsf4Cfg->mode = (uint32_t)dccNsf4Cfg->mode;
        nsf4Cfg->tKnee = (uint32_t)dccNsf4Cfg->u1_knee;
        nsf4Cfg->tnScale[0U] = (uint32_t)dccNsf4Cfg->tn1;
        nsf4Cfg->tnScale[1U] = (uint32_t)dccNsf4Cfg->tn2;
        nsf4Cfg->tnScale[2U] = (uint32_t)dccNsf4Cfg->tn3;

        for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
        {
            for (cnt2 = 0U; cnt2 < NSF4_TN_MAX_SEGMENT; cnt2 ++)
            {
                nsf4Cfg->tnCurve[cnt1][cnt2].posX  = dccNsf4Cfg->noise_thr_x[cnt1][cnt2];
                nsf4Cfg->tnCurve[cnt1][cnt2].posY  = dccNsf4Cfg->noise_thr_y[cnt1][cnt2];
                nsf4Cfg->tnCurve[cnt1][cnt2].slope = dccNsf4Cfg->noise_thr_s[cnt1][cnt2];
            }
        }

        lsccCfg->enable         = (uint32_t)dccNsf4Cfg->shading_gain;
        lsccCfg->lensCenterX    = (uint32_t)dccNsf4Cfg->shd_x;
        lsccCfg->lensCenterY    = (uint32_t)dccNsf4Cfg->shd_y;
        lsccCfg->tCfg           = (uint32_t)dccNsf4Cfg->shd_t;
        lsccCfg->khCfg          = (uint32_t)dccNsf4Cfg->shd_kh;
        lsccCfg->kvCfg          = (uint32_t)dccNsf4Cfg->shd_kv;
        lsccCfg->gMaxCfg        = (uint32_t)dccNsf4Cfg->shd_gmax;
        lsccCfg->setSel         = (uint32_t)dccNsf4Cfg->shd_set_sel;

        for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
        {
            for (cnt2 = 0U; cnt2 < NSF4_LSCC_MAX_SEGMENT; cnt2 ++)
            {
                 lsccCfg->pwlCurve[cnt1][cnt2].posX = dccNsf4Cfg->shd_lut_x[cnt1][cnt2];
                 lsccCfg->pwlCurve[cnt1][cnt2].posY = dccNsf4Cfg->shd_lut_y[cnt1][cnt2];
                 lsccCfg->pwlCurve[cnt1][cnt2].slope = dccNsf4Cfg->shd_lut_s[cnt1][cnt2];
             }
        }

        /* Override gains from AWB results */
        if (NULL != ae_awb_res)
        {
            if(1u == ae_awb_res->awb_valid)
            {
                for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
                {
                    nsf4Cfg->gains[cnt1] = ae_awb_res->wb_gains[cnt1];
                }
            }
        }
        else
        {
            for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
            {
                nsf4Cfg->gains[cnt1] = (uint32_t)dccNsf4Cfg->wb_gains[cnt1];
            }
        }
    }
    else
    {
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

        lsccCfg->enable         = (uint32_t)dccNsf4Cfg->shading_gain;
        lsccCfg->lensCenterX    = (uint32_t)dccNsf4Cfg->shd_x;
        lsccCfg->lensCenterY    = (uint32_t)dccNsf4Cfg->shd_y;
        lsccCfg->tCfg           = (uint32_t)dccNsf4Cfg->shd_t;
        lsccCfg->khCfg          = (uint32_t)dccNsf4Cfg->shd_kh;
        lsccCfg->kvCfg          = (uint32_t)dccNsf4Cfg->shd_kv;
        lsccCfg->gMaxCfg        = (uint32_t)dccNsf4Cfg->shd_gmax;
        lsccCfg->setSel         = (uint32_t)dccNsf4Cfg->shd_set_sel;

        for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
        {
            for (cnt2 = 0U; cnt2 < NSF4_LSCC_MAX_SEGMENT; cnt2 ++)
            {
                 lsccCfg->pwlCurve[cnt1][cnt2].posX = dccNsf4Cfg->shd_lut_x[cnt1][cnt2];
                 lsccCfg->pwlCurve[cnt1][cnt2].posY = dccNsf4Cfg->shd_lut_y[cnt1][cnt2];
                 lsccCfg->pwlCurve[cnt1][cnt2].slope = dccNsf4Cfg->shd_lut_s[cnt1][cnt2];
             }
        }

        /* Override gains from AWB results */
        if (NULL != ae_awb_res)
        {
            for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
            {
                nsf4Cfg->gains[cnt1] = ae_awb_res->wb_gains[cnt1];
            }
        }
        else
        {
            for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
            {
                nsf4Cfg->gains[cnt1] = (uint32_t)dccNsf4Cfg->wb_gains[cnt1];
            }
        }
        lsccCfg->enable         = 0;
        lsccCfg->lensCenterX    = 0;
        lsccCfg->lensCenterY    = 0;
        lsccCfg->tCfg           = 0;
        lsccCfg->khCfg          = 0;
        lsccCfg->kvCfg          = 0;
        lsccCfg->gMaxCfg        = 0;
        lsccCfg->setSel         = 0;

        /* Override gains from AWB results */
        if (NULL != ae_awb_res)
        {
            for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
            {
                nsf4Cfg->gains[cnt1] = ae_awb_res->wb_gains[cnt1];
            }
        }
        else
        {
            for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
            {
                nsf4Cfg->gains[cnt1] = 512u;
            }
        }

    }

    vissObj->vissCfgRef.nsf4Cfg = nsf4Cfg;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;

}

static void tivxVpacVissDccMapHistParams(tivxVpacVissObj *vissObj)
{
    Fcp_HistConfig  *histCfg;

    if (NULL != vissObj)
    {
        histCfg = &vissObj->vissCfg.histCfg;

        histCfg->enable = (uint32_t) FALSE;
        histCfg->input = FCP_HIST_IN_SEL_COLOR_RED;
        histCfg->roi.cropStartX = 0u;
        histCfg->roi.cropStartY = 0u;
        histCfg->roi.cropWidth = 500u;
        histCfg->roi.cropHeight = 500u;

        vissObj->vissCfgRef.histCfg = histCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapRGBLutParams(tivxVpacVissObj *vissObj)
{
    Fcp_YuvSatLutConfig *yuvSatLutCfg;

    if (NULL != vissObj)
    {
        yuvSatLutCfg = &vissObj->vissCfg.yuvSatLutCfg;

        yuvSatLutCfg->lumaInputBits = 10u;
        yuvSatLutCfg->enableLumaLut = (uint32_t) FALSE;
        yuvSatLutCfg->lumaLutAddr = gflexcc_lut_12to8;
        yuvSatLutCfg->enableChromaLut = (uint32_t) FALSE;
        yuvSatLutCfg->chromaLutAddr = gflexcc_lut_12to8;
        yuvSatLutCfg->enableSaturLut = (uint32_t) FALSE;
        yuvSatLutCfg->saturLutAddr = gflexcc_lut_12to8;

        vissObj->vissCfgRef.yuvSatLutCfg = yuvSatLutCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapGammaParams(tivxVpacVissObj *vissObj)
{
    Fcp_GammaConfig *gamma;

    if (NULL != vissObj)
    {
        gamma = &vissObj->vissCfg.gammaCfg;

        gamma->enable = (uint32_t)TRUE;
        gamma->outClip = 10u;
        gamma->tableC1 = gflexcc_contrast_lut;
        gamma->tableC2 = gflexcc_contrast_lut;
        gamma->tableC3 = gflexcc_contrast_lut;

        vissObj->vissCfgRef.gamma = gamma;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapRGB2HSVParams(tivxVpacVissObj *vissObj)
{
    Fcp_Rgb2HsvConfig  *r2h = NULL;

    if (NULL != vissObj)
    {
        r2h = &vissObj->vissCfg.rgb2hsvCfg;

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

        vissObj->vissCfgRef.rgb2Hsv = r2h;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapRGB2YUVParams(tivxVpacVissObj *vissObj)
{
    Fcp_Rgb2YuvConfig  *r2y = NULL;

    if (NULL != vissObj)
    {
        r2y = &vissObj->vissCfg.rgb2yuvCfg;

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

        vissObj->vissCfgRef.rgb2yuv = r2y;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t            cnt1, cnt2;
    Fcp_CcmConfig      *ccmCfg = NULL;
    ccmCfg = &vissObj->vissCfg.ccmCfg;

    if ((vissObj->dcc_out_prms.useCcmCfg != 0) && (NULL != ae_awb_res))
    {
        int color_temp = ae_awb_res->color_temperature;
        int n_regions = vissObj->dcc_out_prms.ipipeNumRgb2Rgb1Inst;
        iss_ipipe_rgb2rgb ccm_int;
        iss_ipipe_rgb2rgb *rgb2rgb = &ccm_int;

        dcc_interp_CCM(
            vissObj->dcc_out_prms.phPrmsRgb2Rgb1,
            n_regions,
            color_temp,
            vissObj->dcc_out_prms.ipipeRgb2Rgb1Cfg,
            rgb2rgb);

        for (cnt1 = 0u; cnt1 < FCP_MAX_CCM_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_CCM_COEFF_IN_RAW; cnt2 ++)
            {
                ccmCfg->weights[cnt1][cnt2] = rgb2rgb->matrix[cnt1][cnt2];
            }
            ccmCfg->offsets[cnt1] = rgb2rgb->offset[cnt1];
        }

        vissObj->vissCfgRef.ccm = ccmCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
    else
    {
        /* Map DCC Output Config to FVID2 Driver Config */
        for (cnt1 = 0u; cnt1 < FCP_MAX_CCM_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_CCM_COEFF_IN_RAW; cnt2 ++)
            {
                ccmCfg->weights[cnt1][cnt2] = 0;
            }
            ccmCfg->offsets[cnt1] = 0;
        }
        // Note: hardcoding
        ccmCfg->weights[0][0] = 256;
        ccmCfg->weights[1][1] = 256;
        ccmCfg->weights[2][2] = 256;
        vissObj->vissCfgRef.ccm = ccmCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t                    cnt;
    dcc_parser_output_params_t *dcc_out_prms;
    Rfe_PwlConfig              *pwlCfg;

    dcc_out_prms = &vissObj->dcc_out_prms;

    pwlCfg = &vissObj->vissCfg.pwlCfg1;
    for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt ++)
    {
        pwlCfg->offset[cnt] = dcc_out_prms->vissBLC.l_dcoffset[cnt];
    }
    vissObj->vissCfgRef.lPwlCfg = pwlCfg;

    pwlCfg = &vissObj->vissCfg.pwlCfg2;
    for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt ++)
    {
        pwlCfg->offset[cnt] = dcc_out_prms->vissBLC.s_dcoffset[cnt];
    }
    vissObj->vissCfgRef.sPwlCfg = pwlCfg;

    pwlCfg = &vissObj->vissCfg.pwlCfg3;
    for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt ++)
    {
        pwlCfg->offset[cnt] = dcc_out_prms->vissBLC.vs_dcoffset[cnt];
    }
    vissObj->vissCfgRef.vsPwlCfg = pwlCfg;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

void tivxVpacVissDccMapEeParams(tivxVpacVissObj *vissObj)
{
    Fcp_EeConfig *eeCfg = NULL;

    eeCfg = &vissObj->vissCfg.eeCfg;

    if (NULL != vissObj)
    {
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

        vissObj->vissCfgRef.eeCfg = eeCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj)
{
    uint32_t                 cnt;
    Glbce_Config            *glbceCfg = NULL;
    Glbce_PerceptConfig     *prcptCfg = NULL;

    glbceCfg = &vissObj->vissCfg.glbceCfg;
    if (NULL != vissObj)
    {
        if (0U == vissObj->dcc_out_prms.useVissGlbceCfg)
        {
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
            memcpy(glbceCfg->asymLut, gGlbceAsymTbl, GLBCE_ASYMMETRY_LUT_SIZE * 4U);

            vissObj->vissCfgRef.glbceCfg = glbceCfg;

            prcptCfg = &vissObj->vissCfg.fwdPrcpCfg;
            prcptCfg->enable = (uint32_t)FALSE;

            for (cnt = 0u; cnt < GLBCE_PERCEPT_LUT_SIZE; cnt++)
            {
                prcptCfg->table[cnt] = gGlbceFwdPrcptTbl[cnt];
            }

            prcptCfg = &vissObj->vissCfg.revPrcpCfg;
            prcptCfg->enable = (uint32_t)FALSE;

            for (cnt = 0u; cnt < GLBCE_PERCEPT_LUT_SIZE; cnt++)
            {
                prcptCfg->table[cnt] = gGlbceRevPrcptTbl[cnt];
            }
        }
        else
        {
            vissObj->vissCfgRef.glbceCfg = glbceCfg;

            viss_glbce_dcc_cfg_t *dcc_cfg = NULL;
            dcc_cfg = &vissObj->dcc_out_prms.vissGlbceCfg;

            glbceCfg->irStrength = dcc_cfg->strength;
            glbceCfg->intensityVariance = dcc_cfg->intensity_var;
            glbceCfg->spaceVariance = dcc_cfg->space_var;
            glbceCfg->maxSlopeLimit = dcc_cfg->slope_max_lim;
            glbceCfg->minSlopeLimit = dcc_cfg->slope_min_lim;

            glbceCfg->blackLevel = 0;
            glbceCfg->whiteLevel = 65535;
            glbceCfg->brightAmplLimit = 6;
            glbceCfg->darkAmplLimit = 6;
            glbceCfg->dither = GLBCE_NO_DITHER;

            memcpy(glbceCfg->asymLut, dcc_cfg->asym_lut, GLBCE_ASYMMETRY_LUT_SIZE * sizeof(uint32_t));

            prcptCfg = &vissObj->vissCfg.fwdPrcpCfg;
            prcptCfg->enable = dcc_cfg->fwd_prcpt_en;
            memcpy(prcptCfg->table, dcc_cfg->fwd_prcpt_lut, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));

            prcptCfg = &vissObj->vissCfg.revPrcpCfg;
            prcptCfg->enable = (uint32_t) dcc_cfg->rev_prcpt_en;
            memcpy(prcptCfg->table, dcc_cfg->rev_prcpt_lut, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }

}

static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj)
{
    uint32_t cnt;
    Fcp_CfaConfig *cfaCfg;
    viss_ipipe_cfa_flxd   * dcc_cfa_cfg = NULL;

    if (NULL != vissObj)
    {
        cfaCfg = &vissObj->vissCfg.cfaCfg;
        dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCFACfg);
    }

    if (NULL != dcc_cfa_cfg)
    {
        memcpy(cfaCfg->coeff, dcc_cfa_cfg->FirCoefs, FCP_MAX_CFA_COEFF * sizeof(cfaCfg->coeff[0]));

        for (cnt = 0u; cnt < FCP_MAX_COLOR_COMP; cnt ++)
        {
            cfaCfg->bypass[cnt]             = (uint32_t)FALSE;
            cfaCfg->coreSel[cnt]             = dcc_cfa_cfg->bitMaskSel[cnt];
            cfaCfg->coreBlendMode[cnt]     = dcc_cfa_cfg->blendMode[cnt];

            cfaCfg->gradHzPh[0u][cnt]     = dcc_cfa_cfg->Set0GradHzMask[cnt];
            cfaCfg->gradHzPh[1u][cnt]     = dcc_cfa_cfg->Set1GradHzMask[cnt];

            cfaCfg->gradVtPh[0u][cnt]     = dcc_cfa_cfg->Set0GradVtMask[cnt];
            cfaCfg->gradVtPh[1u][cnt]     = dcc_cfa_cfg->Set1GradVtMask[cnt];

            cfaCfg->intsBitField[0u][cnt] = dcc_cfa_cfg->Set0IntensityMask[cnt];
            cfaCfg->intsBitField[1u][cnt] = dcc_cfa_cfg->Set1IntensityMask[cnt];

            cfaCfg->intsShiftPh[0u][cnt]  = dcc_cfa_cfg->Set0IntensityShift[cnt];
            cfaCfg->intsShiftPh[1u][cnt]  = dcc_cfa_cfg->Set1IntensityShift[cnt];
        }
        for (cnt = 0u; cnt < FCP_CFA_MAX_SET_THR; cnt++)
        {
            cfaCfg->thr[0u][cnt] = dcc_cfa_cfg->Set0Thr[cnt];
            cfaCfg->thr[1u][cnt] = dcc_cfa_cfg->Set1Thr[cnt];
        }

        {
            Vhwa_LutConfig *lut16to12Cfg = &vissObj->vissCfg.cfaLut16to12Cfg;
            lut16to12Cfg->enable    = dcc_cfa_cfg->lut_enable;
            lut16to12Cfg->inputBits = dcc_cfa_cfg->bitWidth;

            memcpy(gcfa_lut_16to12, dcc_cfa_cfg->ToneLut, sizeof(uint32_t) * (uint32_t)FLXD_LUT_SIZE);
            lut16to12Cfg->tableAddr = gcfa_lut_16to12;
            vissObj->vissCfgRef.cfaLut16to12Cfg = lut16to12Cfg;
        }

        vissObj->vissCfgRef.cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

void tivxVpacVissDccMapFlexCFAParamsDefaults(tivxVpacVissObj *vissObj)
{
    uint32_t cnt;
    Fcp_CfaConfig *cfaCfg;

    cfaCfg = &vissObj->vissCfg.cfaCfg;
    if (NULL != vissObj)
    {
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

        vissObj->vissCfgRef.cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res)
{
    if (NULL != vissObj)
    {
        tivxVpacVissDccMapCCMParams(vissObj, ae_awb_res);
        tivxVpacVissDccMapRGB2YUVParams(vissObj);
        tivxVpacVissDccMapRGB2HSVParams(vissObj);
        tivxVpacVissDccMapGammaParams(vissObj);
        tivxVpacVissDccMapRGBLutParams(vissObj);
        tivxVpacVissDccMapHistParams(vissObj);
        tivxVpacVissDccMapEeParams(vissObj);
    }
}


static void tivxVpacVissDccMapMergeParams(tivxVpacVissObj *vissObj,
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

static void tivxVpacVissDccMapRfeLutParams(tivxVpacVissObj *vissObj, uint32_t lut_id)
{
    uint32_t                     cnt;
    Vhwa_LutConfig              *lutCfg;
    dcc_parser_output_params_t  *dcc_out_prms;

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
            dcc_out_prms = &vissObj->dcc_out_prms;

            if (1u == vissObj->dcc_out_prms.issH3aMuxLuts.enable)
            {
                lutCfg->enable    = 1u;
                lutCfg->inputBits = 16u;
                lutCfg->clip      = 1023u;

                for (cnt = 0u; cnt < RFE_H3A_COMP_LUT_SIZE; cnt ++)
                {
                    gH3aLut[cnt] =
                        dcc_out_prms->issH3aMuxLuts.h3a_mux_lut[0u][cnt];
                }

                lutCfg->tableAddr = gH3aLut;
            }
            else
            {
                lutCfg->enable = 0u;
            }

            vissObj->vissCfgRef.h3aLutCfg = lutCfg;
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

static void tivxVpacVissDccMapDpcOtfParams(tivxVpacVissObj *vissObj)
{
    Rfe_DpcOtfConfig *dpcOtfCfg;

    if (NULL != vissObj)
    {
        dpcOtfCfg = &vissObj->vissCfg.dpcOtfCfg;

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

        vissObj->vissCfgRef.dpcOtf  = dpcOtfCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapDpcLutParams(tivxVpacVissObj *vissObj)
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

static void tivxVpacVissDccMapLscParams(tivxVpacVissObj *vissObj)
{
    Rfe_LscConfig *lscCfg;

    if (NULL != vissObj)
    {
        lscCfg = &vissObj->vissCfg.lscCfg;

        lscCfg->enable          = (uint32_t)FALSE;
        lscCfg->gainFmt         = RFE_LSC_GAIN_FMT_U8Q8;
        lscCfg->horzDsFactor    = RFE_LSC_DS_FACTOR_32;
        lscCfg->vertDsFactor    = RFE_LSC_DS_FACTOR_16;
        lscCfg->tableAddr       = grawfe_lsc_tbl;
        lscCfg->numTblEntry     = 0x0u;

        vissObj->vissCfgRef.lscCfg = lscCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapWb2Params(tivxVpacVissObj *vissObj)
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

static void tivxVpacVissDccMapPwlParams(tivxVpacVissObj *vissObj,
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
            pwlCfg->offset[0u]  = 0;   //S16
            pwlCfg->offset[1u]  = 0;   //S16
            pwlCfg->offset[2u]  = 0;   //S16
            pwlCfg->offset[3u]  = 0;   //S16
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

            if (1U == vissObj->dcc_out_prms.issRfeDecompand.enable)
            {
                int32_t i;
                dcc_parser_output_params_t  *dcc_out_prms = &vissObj->dcc_out_prms;
                lutCfg->enable    = dcc_out_prms->issRfeDecompand.enable;
                pwlCfg->mask      = dcc_out_prms->issRfeDecompand.mask;
                pwlCfg->shift     = dcc_out_prms->issRfeDecompand.shift;
                lutCfg->inputBits = dcc_out_prms->issRfeDecompand.bit_depth;
                lutCfg->clip      = dcc_out_prms->issRfeDecompand.clip;
                for (i = 0; i < FLXD_LUT_SIZE; i++)
                {
                    grawfe_pwl_vshort_lut[i] = dcc_out_prms->issRfeDecompand.lut[i];
                }
            }

            vissObj->vissCfgRef.vsPwlCfg = pwlCfg;
            vissObj->vissCfgRef.vsLutCfg = lutCfg;
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

void tivxVpacVissDccMapRfeParams(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissDccMapPwlParams(vissObj, 0u);
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissDccMapPwlParams(vissObj, 1u);
        /* Parse Long exposure PWL Parameters */
        tivxVpacVissDccMapPwlParams(vissObj, 2u);

        tivxVpacVissDccMapMergeParams(vissObj, 0u);
        tivxVpacVissDccMapMergeParams(vissObj, 1u);

        /* Parse Lut for 20 to 16 conversion */
        tivxVpacVissDccMapRfeLutParams(vissObj, 0u);

        /* Parse DPC OTF Parameters */
        tivxVpacVissDccMapDpcOtfParams(vissObj);
        /* Parse DPC OTF Parameters */
        tivxVpacVissDccMapDpcLutParams(vissObj);
        /* Parse LSC Parameters */
        tivxVpacVissDccMapLscParams(vissObj);
        /* Parse WB2 Parameters */
        tivxVpacVissDccMapWb2Params(vissObj);

        /* Parse H3A Source Parameters */
        tivxVpacVissDccMapRfeLutParams(vissObj, 1u);
    }
}

