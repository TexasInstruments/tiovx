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
static void tivxVpacVissDccMapH3aLutParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapNsf4Params(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxVpacVissInitDcc(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vissObj->sensor_dcc_id = vissPrms->sensor_dcc_id;

    /* Calculate the number of bytes required for
     * storing DCC output */
    vissObj->dcc_out_numbytes = calc_dcc_outbuf_size();

    /* Allocate DCC output buffer */
    vissObj->dcc_out_buf = (uint8_t *)tivxMemAlloc(
        vissObj->dcc_out_numbytes, (vx_enum)TIVX_MEM_EXTERNAL);

    if (NULL == vissObj->dcc_out_buf)
    {
        VX_PRINT(VX_ZONE_ERROR, "failed to allocate %d bytes !!!\n", vissObj->dcc_out_numbytes);
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
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    dcc_status = 0;
    dcc_parser_input_params_t  dcc_in;
    dcc_parser_input_params_t  *dcc_in_prms;
    dcc_parser_output_params_t *dcc_out_prms;

    if (NULL != dcc_buf_desc)
    {
        dcc_in_prms = &dcc_in;
        dcc_in_prms->dcc_buf = NULL;
        dcc_in_prms->dcc_buf_size = 0;
        dcc_in_prms->cameraId = vissObj->sensor_dcc_id;

        dcc_out_prms = &vissObj->dcc_out_prms;

        dcc_in_prms->dcc_buf_size = dcc_buf_desc->mem_size;
        dcc_in_prms->dcc_buf = tivxMemShared2TargetPtr(&dcc_buf_desc->mem_ptr);

        if(NULL != dcc_in_prms->dcc_buf)
        {
            tivxCheckStatus(&status, tivxMemBufferMap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            dcc_status = dcc_update(dcc_in_prms, dcc_out_prms);

            if (0 != dcc_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "dcc_update Failed !!!\n");
                status = (vx_status)VX_FAILURE;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(dcc_in_prms->dcc_buf,
                dcc_buf_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect DCC Input Buf memory !!!\n");
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
        /* TODO: Use shift when H3A Lut is not enabled */
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
    tivxVpacVissConfig              *vsCfg;
    Rfe_GainOfstConfig              *wbCfg;
    dcc_parser_input_params_t       dcc_in;
    dcc_parser_input_params_t       *dcc_in_prms;

    vsCfg = &vissObj->vissCfg;
    dcc_in_prms = &dcc_in;

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
    else
    {
        /* In case of multi-instance, these need to be updated in
         * driver (required until there is better ctx restore mechanism)*/
        vissObj->vissCfgRef.ccm = &vissObj->vissCfg.ccmCfg;
        if (0u == vissObj->bypass_nsf4)
        {
            if(1u == aewb_result->awb_valid)
            {
                int i;
                for (i = 0; i < 4; i ++)
                {
                    vissObj->vissCfg.nsf4Cfg.gains[i] = aewb_result->wb_gains[i];
                }
            }

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

static void tivxVpacVissDccMapH3aLutParams(tivxVpacVissObj *vissObj)
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
        h3aLutCfg->tableAddr = vissObj->dcc_table_ptr.h3aLut;

        /* TODO: Select the h3a_mux_lut based on frame count. */
        /* Can't use memcpy since dcc is 16 bits and driver is 32 bits per entry */
        for (cnt = 0U; cnt < RFE_H3A_COMP_LUT_SIZE; cnt ++)
        {
            h3aLutCfg->tableAddr[cnt] = dccH3aLutCfg->h3a_mux_lut[0U][cnt];
        }
    }
    else
    {
        h3aLutCfg->enable = 0u;
    }

    vissObj->vissCfgRef.h3aLutCfg = h3aLutCfg;

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
    viss_nsf4          phNsf4Cfg;
    viss_nsf4          *dccNsf4Cfg = &phNsf4Cfg;
    uint32_t n_regions = 0;
    int32_t dcc_status = -1;

    n_regions = vissObj->dcc_out_prms.vissNumNSF4Inst;

    if((NULL != ae_awb_res) && (ae_awb_res->ae_valid))
    {
        dcc_status = dcc_search_NSF4(
            vissObj->dcc_out_prms.phPrmsNSF4,
            n_regions,
            ae_awb_res->analog_gain,
            vissObj->dcc_out_prms.vissNSF4Cfg,
            dccNsf4Cfg);

        if(dcc_status < 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "dcc_search_NSF4 failed for analog_gain = 0x%x !!!\n", ae_awb_res->analog_gain);
        }
    }

    nsf4Cfg = &vissObj->vissCfg.nsf4Cfg;
    lsccCfg = &nsf4Cfg->lsccCfg;

    /* Map DCC Output Config to FVID2 Driver Config */

    /* TODO: Does shading gain map to lscc enable? */

    if ((NULL != vissObj) && (0 != vissObj->dcc_out_prms.useNsf4Cfg))
    {
        if(0==dcc_status)
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

        vissObj->vissCfgRef.nsf4Cfg = nsf4Cfg;

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

    if (vissObj->dcc_out_prms.useCcmCfg != 0)
    {
        if(NULL != ae_awb_res)
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
            /*
                Nothing to do. dcc_out_prms has been updated and will take effect
                when tivxVpacVissApplyAEWBParams is called next time
            */
         }
    }
    else
    {
        for (cnt1 = 0u; cnt1 < FCP_MAX_CCM_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_CCM_COEFF_IN_RAW; cnt2 ++)
            {
                ccmCfg->weights[cnt1][cnt2] = 0;
            }
            ccmCfg->offsets[cnt1] = 0;
        }
        /* Note: hardcoding */
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

static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj)
{
    Glbce_Config            *glbceCfg = NULL;
    Glbce_PerceptConfig     *prcptCfg = NULL;

    glbceCfg = &vissObj->vissCfg.glbceCfg;
    if (NULL != vissObj)
    {
        if (0U != vissObj->dcc_out_prms.useVissGlbceCfg)
        {
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

            vissObj->vissCfgRef.glbceCfg = glbceCfg;

            prcptCfg = &vissObj->vissCfg.fwdPrcpCfg;
            prcptCfg->enable = dcc_cfg->fwd_prcpt_en;
            memcpy(prcptCfg->table, dcc_cfg->fwd_prcpt_lut, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));
            vissObj->vissCfgRef.fwdPrcpCfg = prcptCfg;

            prcptCfg = &vissObj->vissCfg.revPrcpCfg;
            prcptCfg->enable = (uint32_t) dcc_cfg->rev_prcpt_en;
            memcpy(prcptCfg->table, dcc_cfg->rev_prcpt_lut, GLBCE_PERCEPT_LUT_SIZE * sizeof(uint32_t));
            vissObj->vissCfgRef.revPrcpCfg = prcptCfg;

            /* Setting config flag to 1,
             * assumes caller protects this flag */
            vissObj->isConfigUpdated = 1U;
        }
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
            cfaCfg->bypass[cnt]            = (uint32_t)FALSE;
            cfaCfg->coreSel[cnt]           = dcc_cfa_cfg->bitMaskSel[cnt];
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
            lut16to12Cfg->tableAddr = vissObj->dcc_table_ptr.cfa_lut_16to12;

            memcpy(lut16to12Cfg->tableAddr, dcc_cfa_cfg->ToneLut, sizeof(uint32_t) * (uint32_t)FLXD_LUT_SIZE);
            vissObj->vissCfgRef.cfaLut16to12Cfg = lut16to12Cfg;
        }

        vissObj->vissCfgRef.cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res)
{
    if (NULL != vissObj)
    {
        tivxVpacVissDccMapCCMParams(vissObj, ae_awb_res);
    }
}

static void tivxVpacVissDccMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id)
{
    Rfe_PwlConfig   *pwlCfg;
    Vhwa_LutConfig  *lutCfg;

    if (NULL != vissObj)
    {
        if (2u == inst_id)
        {
            pwlCfg = &vissObj->vissCfg.pwlCfg3;
            lutCfg = &vissObj->vissCfg.decomp3Cfg;

            lutCfg->tableAddr = vissObj->dcc_table_ptr.rawfe_pwl_vshort_lut;

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
                    lutCfg->tableAddr[i] = dcc_out_prms->issRfeDecompand.lut[i];
                }
            }

            vissObj->vissCfgRef.vsPwlCfg = pwlCfg;
            vissObj->vissCfgRef.vsLutCfg = lutCfg;
        }

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapLscParams(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        Rfe_LscConfig *lscCfg = &vissObj->vissCfg.lscCfg;
        lscCfg->tableAddr = vissObj->dcc_table_ptr.lsc_lut;
        lscCfg->enable = 0;
        if (1U == vissObj->dcc_out_prms.vissLscCfg.lsc_params.enable)
        {
            viss_lsc_dcc_cfg_t * dcc_cfg = &vissObj->dcc_out_prms.vissLscCfg;
            int32_t len = dcc_cfg->lsc_params.lut_size_in_bytes;

            lscCfg->numTblEntry  = len / 4;
            lscCfg->enable       = dcc_cfg->lsc_params.enable;
            lscCfg->gainFmt      = dcc_cfg->lsc_params.gain_mode_format;
            lscCfg->horzDsFactor = dcc_cfg->lsc_params.gain_mode_m;
            lscCfg->vertDsFactor = dcc_cfg->lsc_params.gain_mode_n;

            if(lscCfg->numTblEntry > RFE_LSC_TBL_SIZE)
            {
                VX_PRINT(VX_ZONE_ERROR, "LSC table length is %d entries, which is greater than RFE_LSC_TBL_SIZE (%d entries)!!!\n", lscCfg->numTblEntry, RFE_LSC_TBL_SIZE);
            }

            memcpy(lscCfg->tableAddr, dcc_cfg->lsc_table, len);
        }

        vissObj->vissCfgRef.lscCfg = lscCfg;

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
        tivxVpacVissDccMapPwlParams(vissObj, 2u);
        tivxVpacVissDccMapLscParams(vissObj);
    }
}
