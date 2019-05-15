/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapRfeParams(tivxVpacVissObj *vissObj);

static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapRGB2YUVParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapRGB2HSVParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapGammaParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapRGBLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapHistParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj);

static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj);

static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj);

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



/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissInitDcc(tivxVpacVissObj *vissObj,
    tivx_vpac_viss_params_t *vissPrms)
{
    vx_status status = VX_SUCCESS;

    vissObj->dcc_in_prms.analog_gain = 1000;
    vissObj->dcc_in_prms.cameraId = vissPrms->sensor_dcc_id;
    vissObj->dcc_in_prms.color_temparature = 5000;
    vissObj->dcc_in_prms.exposure_time = 33333;

    /* Calculate the number of bytes required for
     * storing DCC output */
    vissObj->dcc_out_numbytes = calc_dcc_outbuf_size();

    /* Allocate DCC output buffer */
    vissObj->dcc_out_buf = (uint8_t *)tivxMemAlloc(
        vissObj->dcc_out_numbytes, TIVX_MEM_EXTERNAL);

    if (NULL == vissObj->dcc_out_buf)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacVissInitDcc: dcc_update Failed !!!\n");
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

void tivxVpacVissDeInitDcc(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj->dcc_out_buf)
    {
        tivxMemFree(vissObj->dcc_out_buf, vissObj->dcc_out_numbytes,
            TIVX_MEM_EXTERNAL);
    }
}

vx_status tivxVpacVissSetParamsFromDcc(tivxVpacVissObj *vissObj,
    tivx_obj_desc_user_data_object_t *dcc_buf_desc,
    tivx_ae_awb_params_t *ae_awb_res)
{
    vx_status                   status = VX_SUCCESS;
    int                         dcc_status;
    dcc_parser_input_params_t  *dcc_in_prms;
    dcc_parser_output_params_t *dcc_out_prms;

    if (NULL != dcc_buf_desc)
    {
        dcc_in_prms = &vissObj->dcc_in_prms;
        dcc_out_prms = &vissObj->dcc_out_prms;

        dcc_in_prms->dcc_buf_size = dcc_buf_desc->mem_size;
        dcc_in_prms->dcc_buf = tivxMemShared2TargetPtr(
            dcc_buf_desc->mem_ptr.shared_ptr,
            dcc_buf_desc->mem_ptr.mem_heap_region);

        if(NULL != dcc_in_prms->dcc_buf)
        {
            tivxMemBufferMap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            dcc_status = Dcc_Create(dcc_out_prms, vissObj->dcc_out_buf);
            if (0 == dcc_status)
            {
                dcc_status = dcc_update(dcc_in_prms, dcc_out_prms);

                if (0 != dcc_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacVissSetParamsFromDcc: dcc_update Failed !!!\n");
                    status = VX_FAILURE;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacVissSetParamsFromDcc: Dcc_Create Failed !!!\n");
                status = VX_FAILURE;
            }

            tivxMemBufferUnmap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissSetParamsFromDcc: Incorrect DCC Input Buf memory !!!\n");
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxVpacVissDccMapRfeParams(vissObj);
            tivxVpacVissDccMapNsf4Params(vissObj, ae_awb_res);
            tivxVpacVissDccMapH3aParams(vissObj, ae_awb_res);
            tivxVpacVissDccMapH3aLutParams(vissObj);
            tivxVpacVissDccMapGlbceParams(vissObj);
            tivxVpacVissDccMapFlexCFAParams(vissObj);
            tivxVpacVissDccMapFlexCCParams(vissObj);

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

        vissObj->vissCfgRef.h3aLutCfg = h3aLutCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

void tivxVpacVissSetH3aSrcParams(tivxVpacVissObj *vissObj,
    tivx_vpac_viss_params_t *vissPrms)
{
    Rfe_H3aInConfig     *inCfg;

    if ((0U != vissPrms->h3a_aewb_af_mode) &&
        (vissPrms->h3a_in != vissObj->lastH3aInSrc))
    {
        inCfg = &vissObj->vissCfg.h3aInCfg;

        inCfg->inSel = vissPrms->h3a_in;

        /* TODO: Assumption that lut is used for 16 to 10 bit conversion */
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
    vx_status                        status = VX_SUCCESS;
    int                              dcc_status;
    tivxVpacVissConfig              *vsCfg;
    Rfe_GainOfstConfig              *wbCfg;
    dcc_parser_input_params_t       *dcc_in_prms;
    dcc_parser_output_params_t      *dcc_out_prms;

    vsCfg = &vissObj->vissCfg;
    dcc_in_prms = &vissObj->dcc_in_prms;
    dcc_out_prms = &vissObj->dcc_out_prms;

    if (1u == aewb_result->awb_valid)
    {
        wbCfg = &vsCfg->wbCfg;
        /* apply AWB gains in RAWFE when NSF4 is bypassed */
        if (1u == vissObj->bypass_nsf4)
        {
            wbCfg->gain[0U] = aewb_result->wb_gains[0U];
            wbCfg->gain[1U] = aewb_result->wb_gains[1U];
            wbCfg->gain[2U] = aewb_result->wb_gains[2U];
            wbCfg->gain[3U] = aewb_result->wb_gains[3U];

            vissObj->vissCfgRef.wbCfg = wbCfg;

            /* Setting config flag to 1,
             * assumes caller protects this flag */
            vissObj->isConfigUpdated = 1U;
        }
    }

    if(1u == vissObj->use_dcc)
    {
        dcc_in_prms->analog_gain = aewb_result->analog_gain;
        dcc_in_prms->cameraId = vissObj->sensor_dcc_id;
        dcc_in_prms->color_temparature = aewb_result->color_temperature;
        dcc_in_prms->exposure_time = aewb_result->exposure_time;
        dcc_in_prms->analog_gain = aewb_result->analog_gain;

        dcc_status = dcc_update(dcc_in_prms, dcc_out_prms);

        if (0U != dcc_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacVissApplyAEWBParams: DCC Update Failed !!!\n");
            status = VX_FAILURE;
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

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

static void tivxVpacVissDccMapNsf4Params(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t            cnt1, cnt2;
    Nsf4v_Config       *nsf4Cfg = NULL;
    Nsf4_LsccConfig    *lsccCfg = NULL;
    viss_nsf4          *dccNsf4Cfg = NULL;

    nsf4Cfg = &vissObj->vissCfg.nsf4Cfg;
    lsccCfg = &nsf4Cfg->lsccCfg;
    dccNsf4Cfg = &vissObj->dcc_out_prms.vissNSF4Cfg;

    /* Map DCC Output Config to FVID2 Driver Config */

    /* TODO: Does shading gain map to lscc enable? */

    nsf4Cfg->mode = dccNsf4Cfg->mode;
    nsf4Cfg->tKnee = dccNsf4Cfg->u1_knee;
    nsf4Cfg->tnScale[0U] = dccNsf4Cfg->tn1;
    nsf4Cfg->tnScale[1U] = dccNsf4Cfg->tn2;
    nsf4Cfg->tnScale[2U] = dccNsf4Cfg->tn3;

    for (cnt1 = 0U; cnt1 < FVID2_BAYER_COLOR_COMP_MAX; cnt1 ++)
    {
        for (cnt2 = 0U; cnt2 < NSF4_TN_MAX_SEGMENT; cnt2 ++)
        {
            nsf4Cfg->tnCurve[cnt1][cnt2].posX  = dccNsf4Cfg->noise_thr_x[cnt1][cnt2];
            nsf4Cfg->tnCurve[cnt1][cnt2].posY  = dccNsf4Cfg->noise_thr_y[cnt1][cnt2];
            nsf4Cfg->tnCurve[cnt1][cnt2].slope = dccNsf4Cfg->noise_thr_s[cnt1][cnt2];
        }
    }

    lsccCfg->enable         = dccNsf4Cfg->shading_gain;
    lsccCfg->lensCenterX    = dccNsf4Cfg->shd_x;
    lsccCfg->lensCenterY    = dccNsf4Cfg->shd_y;
    lsccCfg->tCfg           = dccNsf4Cfg->shd_t;
    lsccCfg->khCfg          = dccNsf4Cfg->shd_kh;
    lsccCfg->kvCfg          = dccNsf4Cfg->shd_kv;
    lsccCfg->gMaxCfg        = dccNsf4Cfg->shd_gmax;
    lsccCfg->setSel         = dccNsf4Cfg->shd_set_sel;

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
    if ((NULL != ae_awb_res) && (1 == ae_awb_res->awb_valid))
    {
        for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
        {
            nsf4Cfg->gains[cnt1] = ae_awb_res->wb_gains[cnt1];
        }
    }
    else
    {
        for (cnt1 = 0U; cnt1 < NSF4_LSCC_MAX_SET; cnt1 ++)
        {
            nsf4Cfg->gains[cnt1] = dccNsf4Cfg->wb_gains[cnt1];
        }
    }

    vissObj->vissCfgRef.nsf4Cfg = nsf4Cfg;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

static void tivxVpacVissDccMapLut16to12Params(tivxVpacVissObj *vissObj)
{
    Vhwa_LutConfig *lut16to12Cfg;

    if (NULL != vissObj)
    {
        lut16to12Cfg = &vissObj->vissCfg.cfaLut16to12Cfg;

        lut16to12Cfg->enable = (uint32_t)FALSE;
        lut16to12Cfg->inputBits = 12u;
        lut16to12Cfg->tableAddr = gcfa_lut_16to12;

        vissObj->vissCfgRef.cfaLut16to12Cfg = lut16to12Cfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
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
    iss_ipipe_rgb2rgb  *rgb2rgb = NULL;

    ccmCfg = &vissObj->vissCfg.ccmCfg;
    rgb2rgb = vissObj->dcc_out_prms.ipipeRgb2Rgb1Cfg;

    if (NULL != rgb2rgb)
    {
        /* TODO: RGB2RGB matrix seems to be 3x3 whereas CCM is 3x4 */
        /* Map DCC Output Config to FVID2 Driver Config */
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
        pwlCfg->gain[cnt] = dcc_out_prms->vissBLC.l_dcoffset[cnt];
    }
    vissObj->vissCfgRef.lPwlCfg = pwlCfg;

    pwlCfg = &vissObj->vissCfg.pwlCfg2;
    for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt ++)
    {
        pwlCfg->gain[cnt] = dcc_out_prms->vissBLC.s_dcoffset[cnt];
    }
    vissObj->vissCfgRef.sPwlCfg = pwlCfg;

    pwlCfg = &vissObj->vissCfg.pwlCfg3;
    for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt ++)
    {
        pwlCfg->gain[cnt] = dcc_out_prms->vissBLC.vs_dcoffset[cnt];
    }
    vissObj->vissCfgRef.vsPwlCfg = pwlCfg;

    /* Setting config flag to 1,
     * assumes caller protects this flag */
    vissObj->isConfigUpdated = 1U;
}

static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj)
{
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
        memcpy(cfaCfg->coeff, dcc_cfa_cfg->FirCoefs, FCP_MAX_CFA_COEFF * sizeof(int32_t));

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

        vissObj->vissCfgRef.cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        tivxVpacVissDccMapLut16to12Params(vissObj);
        tivxVpacVissDccMapCCMParams(vissObj, NULL);
        tivxVpacVissDccMapRGB2YUVParams(vissObj);
        tivxVpacVissDccMapRGB2HSVParams(vissObj);
        tivxVpacVissDccMapGammaParams(vissObj);
        tivxVpacVissDccMapRGBLutParams(vissObj);
        tivxVpacVissDccMapHistParams(vissObj);
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

        dpcOtfCfg->enable           = (uint32_t)TRUE;
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

        wbCfg->gain[0u] = 694u;
        wbCfg->gain[1u] = 512u;
        wbCfg->gain[2u] = 512u;
        wbCfg->gain[3u] = 1028u;

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
            pwlCfg->offset[0u]   = 0u;
            pwlCfg->offset[1u]   = 0u;
            pwlCfg->offset[2u]   = 0u;
            pwlCfg->offset[3u]   = 0u;
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

            lutCfg->enable       = 1u;
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
            pwlCfg->offset[0u]  = 0u;   //S16
            pwlCfg->offset[1u]  = 0u;   //S16
            pwlCfg->offset[2u]  = 0u;   //S16
            pwlCfg->offset[3u]  = 0u;   //S16
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
            pwlCfg = &vissObj->vissCfg.pwlCfg2;
            lutCfg = &vissObj->vissCfg.decomp2Cfg;

            pwlCfg->mask        = 4095u;
            pwlCfg->shift       = 0u;     /* 3 bits  */
            pwlCfg->offset[0u]  = -127;   //S16
            pwlCfg->offset[1u]  = -127;   //S16
            pwlCfg->offset[2u]  = -127;   //S16
            pwlCfg->offset[3u]  = -127;   //S16
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

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapRfeParams(tivxVpacVissObj *vissObj)
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

