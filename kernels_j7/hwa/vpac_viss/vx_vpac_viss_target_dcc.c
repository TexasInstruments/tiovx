/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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
#include <math.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TIVX_DCC_IR_REMAP_LUT_SIZE 609

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
static int calc_dcc_gain_EV(int analog_gain_linear_Q10);
static void tivxVpacVissDccMapYeeParams(tivxVpacVissObj *vissObj, const tivx_ae_awb_params_t *ae_awb_res,
    uint32_t fcp_index);
static void tivxVpacVissDccMapBlc(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res);
static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index);
static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index);
static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj,
    uint32_t fcp_index);
static void tivxVpacVissDccMapGlbceParams(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapPwlParams(tivxVpacVissObj *vissObj,
    uint32_t inst_id);
static void tivxVpacVissDccInitDpc(tivxVpacVissObj *vissObj);
static void tivxVpacVissDccMapDpcParams(tivxVpacVissObj *vissObj, const tivx_ae_awb_params_t *ae_awb_res);
#if defined(VPAC3) || defined(VPAC3L)
static void tivxVpacVissDccMapCacParams(tivxVpacVissObj *vissObj);
#endif
static void tivxVpacVissDccMapFcpParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res);
#if defined(VPAC3L)
static void tivxVpacVissDccMapPcidParams(tivxVpacVissObj *vissObj);
#endif


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

static void tivxVpacVissDccMapWb1(tivxVpacVissObj *vissObj)
{
    uint32_t                    cnt;
    dcc_parser_output_params_t *dcc_out_prms = &vissObj->dcc_out_prms;
    Rfe_PwlConfig              *pwlCfg = &vissObj->vissCfg.pwlCfg3;

    if (dcc_out_prms->useVissRawfeWb1VsCfg)
    {
        for (cnt = 0U; cnt < RFE_MAX_COLOR_COMP; cnt++)
        {
            pwlCfg->gain[cnt] = dcc_out_prms->vissRawfeWb1VsCfg.gain[cnt];
        }
        vissObj->vissCfgRef.vsPwlCfg = pwlCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
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
            tivxVpacVissDccInitDpc(vissObj);
            tivxVpacVissDccMapDpcParams(vissObj, ae_awb_res);
#if defined(VPAC3) || defined(VPAC3L)
            tivxVpacVissDccMapCacParams(vissObj);
#endif
            if (NULL != h3a_out_desc)
            {
                tivxVpacVissDccMapH3aParams(vissObj, ae_awb_res);
            }
            tivxVpacVissDccMapH3aLutParams(vissObj);
            tivxVpacVissDccMapGlbceParams(vissObj);
            tivxVpacVissDccMapFcpParams(vissObj, ae_awb_res);
            tivxVpacVissDccMapBlc(vissObj, ae_awb_res);
            tivxVpacVissDccMapWb1(vissObj);
#if defined(VPAC3L)
            tivxVpacVissDccMapPcidParams(vissObj);
#endif
        }
    }

    return (status);
}

void tivxVpacVissSetH3aSrcParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms)
{
    Rfe_H3aInConfig     *inCfg;

    if (((vx_bool)vx_true_e == vissObj->h3a_out_enabled) 
    #ifndef AM62A
    && (vissPrms->h3a_in != vissObj->lastH3aInSrc)
    #endif
    )
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
    uint32_t                        fcp_index;

    vsCfg = &vissObj->vissCfg;
    dcc_in_prms = &dcc_in;

    wbCfg = &vsCfg->wbCfg;

#if defined(VPAC3L)
    tivxVpacVissDccMapPcidParams(vissObj);
#endif

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
        tivxVpacVissDccMapDpcParams(vissObj, aewb_result);

        /* Apply DCC Output and update CCM */
        for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
        {
            tivxVpacVissDccMapCCMParams(vissObj, aewb_result, fcp_index);
        }

        /* Update WB Gains in NSF4 if it is enabled */
        if (0u == vissObj->bypass_nsf4)
        {
            tivxVpacVissDccMapNsf4Params(vissObj, aewb_result);
        }

        for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
        {
            tivxVpacVissDccMapYeeParams(vissObj, aewb_result, fcp_index);
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
        for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
        {
            vissObj->vissCfgRef.fcpCfg[fcp_index].ccm = &vissObj->vissCfg.fcpCfg[fcp_index].ccmCfg;
        }

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

    if (NULL != vissObj)
    {
#if defined(VPAC3) || defined(VPAC3L) 
        {
            int k;
            viss_rawhist_dcc_cfg_t *dccCfg = &vissObj->dcc_out_prms.vissRawhistCfg;
            Nsf4_HistConfig *hwaCfg = &vissObj->vissCfg.nsf4Cfg.histCfg;
            hwaCfg->enable = dccCfg->enable;
            hwaCfg->inBitWidth = dccCfg->lut_bits;
            hwaCfg->phaseSelect = dccCfg->color_en;

            hwaCfg->histLut.enable = dccCfg->lut_en;
            hwaCfg->histLut.tableAddr = vissObj->dcc_table_ptr.raw_hist_lut;
            memcpy(hwaCfg->histLut.tableAddr, dccCfg->rawhist_lut, sizeof(uint16_t)*NSF4_HISTOGRAM_LUT_SIZE);

            for (k = 0; k < NSF4_HIST_MAX_ROI; k++)
            {
                hwaCfg->roi[k].enable = dccCfg->roi_en[k];
                hwaCfg->roi[k].start.startX = dccCfg->roi_h_start[k];
                hwaCfg->roi[k].start.startY = dccCfg->roi_v_start[k];
                hwaCfg->roi[k].end.startX = dccCfg->roi_h_end[k];
                hwaCfg->roi[k].end.startY = dccCfg->roi_v_end[k];
            }
        }
#endif
        n_regions = vissObj->dcc_out_prms.vissNumNSF4Inst;

        if ((NULL != ae_awb_res) &&  (ae_awb_res->ae_valid) && (1 == vissObj->dcc_out_prms.useNsf4Cfg))
        {
            int dcc_gain_ev = calc_dcc_gain_EV(ae_awb_res->analog_gain);
            dcc_status = dcc_search_NSF4(
                vissObj->dcc_out_prms.phPrmsNSF4,
                n_regions,
                dcc_gain_ev,
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

        if ((NULL != vissObj) && (1 == vissObj->dcc_out_prms.useNsf4Cfg))
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

            vissObj->vissCfgRef.nsf4Cfg = nsf4Cfg;

            /* Setting config flag to 1,
             * assumes caller protects this flag */
            vissObj->isConfigUpdated = 1U;
        }
    }
}

static int calc_dcc_gain_EV(int analog_gain_linear_Q10)
{
    int ag = analog_gain_linear_Q10;
    if (ag <= 1024) ag = 1024;
    int dcc_gain_in_ev = (int)(10 * (log(ag) / log(2) - 10) + 0.5);
    return dcc_gain_in_ev;
}

static void tivxVpacVissDccMapYeeParams(tivxVpacVissObj *vissObj,
    const tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index)
{
    uint32_t            cnt;
    Fcp_EeConfig        *hwaCfg = NULL;
    viss_yee_dcc_cfg_t  *dccCfg = NULL;
    uint32_t n_regions = 0;
    int32_t dcc_index = 0;

    if ( (NULL != vissObj) && (NULL != ae_awb_res) )
    {
        n_regions = vissObj->dcc_out_prms.vissNumYeeInst;

        if ((0 != ae_awb_res->ae_valid) && (1 == vissObj->dcc_out_prms.useVissYeeCfg))
        {
            int dcc_gain_ev = calc_dcc_gain_EV(ae_awb_res->analog_gain);
            dcc_index = dcc_search_YEE(
                vissObj->dcc_out_prms.phPrmsYee,
                n_regions,
                dcc_gain_ev);
            dccCfg = &vissObj->dcc_out_prms.vissYeeCfg[dcc_index];
        }

        hwaCfg = &vissObj->vissCfg.fcpCfg[fcp_index].eeCfg;

        /* Map DCC Output Config to FVID2 Driver Config */
        if ((NULL != vissObj) && (1 == vissObj->dcc_out_prms.useVissYeeCfg) && (NULL != dccCfg))
        {
            hwaCfg->enable = (uint32_t)dccCfg->enable;
            hwaCfg->yeeShift = (uint32_t)dccCfg->shift_amount;
            hwaCfg->yeeEThr = (uint32_t)dccCfg->threshold_before_lut;
            hwaCfg->yeeMergeSel = (uint32_t)dccCfg->merge_select;
            hwaCfg->haloReductionOn = (uint32_t)dccCfg->halo_reduction_enable;
            hwaCfg->yesEGain = (uint32_t)dccCfg->edge_sharpener_gain;
            hwaCfg->yesEThr1 = (uint32_t)dccCfg->edge_sharpener_hpf_low_thresh;
            hwaCfg->yesEThr2 = (uint32_t)dccCfg->edge_sharpener_hpf_high_thresh;
            hwaCfg->yesGGain = (uint32_t)dccCfg->edge_sharpener_gradient_gain;
            hwaCfg->yesGOfset = (uint32_t)dccCfg->edge_sharpener_gradient_offset;

            for (cnt = 0U; cnt < 9; cnt++)
            {
                hwaCfg->coeff[cnt]  = dccCfg->ee_2d_filter_coeff[cnt];
            }

            hwaCfg->lut = vissObj->dcc_table_ptr.ee_lut;
            for (cnt = 0U; cnt < FCP_EE_LUT_SIZE; cnt++)
            {
                hwaCfg->lut[cnt] = dccCfg->edge_intensity_lut[cnt];
            }

            vissObj->vissCfgRef.fcpCfg[fcp_index].eeCfg = hwaCfg;

            /* Setting config flag to 1,
             * assumes caller protects this flag */
            vissObj->isConfigUpdated = 1U;
        }
    }
}

/* DPC must be enabled at startup time if it is included in DCC */
/* otherwise switching DPC on later will cause image artifacts  */
static void tivxVpacVissDccInitDpc(tivxVpacVissObj *vissObj)
{
    if (NULL != vissObj)
    {
        Rfe_DpcOtfConfig *dpcCfg = &vissObj->vissCfg.dpcOtfCfg;
        dpcCfg->enable = 0;
        if (1U == vissObj->dcc_out_prms.useVissDpcCfg)
        {
            int k;
            dpcCfg->enable = 1;
            for (k = 0; k < 8; k++)
            {
                dpcCfg->threshold[k] = 65535;
                dpcCfg->slope[0] = 0;
            }
#if defined (VPAC3L)
            dpcCfg->cfa_mode         = RFE_CFA_CFG_MODE_4;
            dpcCfg->cfa_phase        = 2;

            dpcCfg->thrx[0u]         = 0u;
            dpcCfg->thrx[1u]         = 60u;
            dpcCfg->thrx[2u]         = 500u;
            dpcCfg->thrx[3u]         = 1000u;
            dpcCfg->thrx[4u]         = 2000u;
            dpcCfg->thrx[5u]         = 4000u;
            dpcCfg->thrx[6u]         = 8000u;
            dpcCfg->thrx[7u]         = 1600u;

            dpcCfg->lut2_threshold[0u]   = 50u;
            dpcCfg->lut2_threshold[1u]   = 50u;
            dpcCfg->lut2_threshold[2u]   = 100u;
            dpcCfg->lut2_threshold[3u]   = 150u;
            dpcCfg->lut2_threshold[4u]   = 250u;
            dpcCfg->lut2_threshold[5u]   = 400u;
            dpcCfg->lut2_threshold[6u]   = 800u;
            dpcCfg->lut2_threshold[7u]   = 1600u;  

            dpcCfg->lut2_slope[0u]    = 0u;
            dpcCfg->lut2_slope[1u]    = 29u;
            dpcCfg->lut2_slope[2u]    = 26u;
            dpcCfg->lut2_slope[3u]    = 26u;
            dpcCfg->lut2_slope[4u]    = 19u;
            dpcCfg->lut2_slope[5u]    = 26u;
            dpcCfg->lut2_slope[6u]    = 26u;
            dpcCfg->lut2_slope[7u]    = 0u;  

            dpcCfg->lut2_thrx[0u]    = 0u;
            dpcCfg->lut2_thrx[1u]    = 60u;
            dpcCfg->lut2_thrx[2u]    = 500u;
            dpcCfg->lut2_thrx[3u]    = 1000u;
            dpcCfg->lut2_thrx[4u]    = 2000u;
            dpcCfg->lut2_thrx[5u]    = 4000u;
            dpcCfg->lut2_thrx[6u]    = 8000u;
            dpcCfg->lut2_thrx[7u]    = 16000u;        

            dpcCfg->lut_map          = 8u; 
#endif
        }

        vissObj->vissCfgRef.dpcOtf = dpcCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapDpcParams(tivxVpacVissObj *vissObj, const tivx_ae_awb_params_t *ae_awb_res)
{
    if (     (NULL != vissObj)
          && (NULL != ae_awb_res)
          && (1 == vissObj->dcc_out_prms.useVissDpcCfg)
#if defined(VPAC3L)
          && (1 == vissObj->dcc_out_prms.useVissDpcExtCfg)
#endif
          && (0 != ae_awb_res->ae_valid)
       )
    {
        uint32_t n_regions = vissObj->dcc_out_prms.vissNumDpcInst;
        int32_t dcc_index = 0;

        int dcc_gain_ev = calc_dcc_gain_EV(ae_awb_res->analog_gain);
        dcc_index = dcc_search_DPC(
            vissObj->dcc_out_prms.phPrmsDpc,
            n_regions,
            dcc_gain_ev);
        viss_dpc_dcc_cfg_t * dccCfg = &vissObj->dcc_out_prms.vissDpcCfg[dcc_index];
#if defined(VPAC3L)
        viss_dpc_ext_dcc_cfg_t *dccExtCfg = &vissObj->dcc_out_prms.vissDpcExtCfg[dcc_index];
#endif

        Rfe_DpcOtfConfig *hwaCfg = &vissObj->vissCfg.dpcOtfCfg;

        hwaCfg->enable = dccCfg->enable;

#if defined(VPAC3L)

        /* threshold[] is synchronous to y of lut0 in VPAC3L */
        hwaCfg->threshold[0]        = dccExtCfg->dpc_lut_0[0][1];
        hwaCfg->threshold[1]        = dccExtCfg->dpc_lut_0[1][1];
        hwaCfg->threshold[2]        = dccExtCfg->dpc_lut_0[2][1];
        hwaCfg->threshold[3]        = dccExtCfg->dpc_lut_0[3][1];
        hwaCfg->threshold[4]        = dccExtCfg->dpc_lut_0[4][1];
        hwaCfg->threshold[5]        = dccExtCfg->dpc_lut_0[5][1];
        hwaCfg->threshold[6]        = dccExtCfg->dpc_lut_0[6][1];
        hwaCfg->threshold[7]        = dccExtCfg->dpc_lut_0[7][1];

        /* slope[] is synchronous to slope of lut0 in VPAC3L */
        hwaCfg->slope[0]            = dccExtCfg->dpc_lut_0[0][2];
        hwaCfg->slope[1]            = dccExtCfg->dpc_lut_0[1][2];
        hwaCfg->slope[2]            = dccExtCfg->dpc_lut_0[2][2];
        hwaCfg->slope[3]            = dccExtCfg->dpc_lut_0[3][2];
        hwaCfg->slope[4]            = dccExtCfg->dpc_lut_0[4][2];
        hwaCfg->slope[5]            = dccExtCfg->dpc_lut_0[5][2];
        hwaCfg->slope[6]            = dccExtCfg->dpc_lut_0[6][2];
        hwaCfg->slope[7]            = dccExtCfg->dpc_lut_0[7][2];

        hwaCfg->cfa_mode            = dccExtCfg->dpc_cfa_mode;
        hwaCfg->cfa_phase           = dccExtCfg->dpc_cfa_phase;

        /* thrx[] is synchronous to x of lut0 in VPAC3L */
        hwaCfg->thrx[0u]            = dccExtCfg->dpc_lut_0[0][0];
        hwaCfg->thrx[1u]            = dccExtCfg->dpc_lut_0[1][0];
        hwaCfg->thrx[2u]            = dccExtCfg->dpc_lut_0[2][0];
        hwaCfg->thrx[3u]            = dccExtCfg->dpc_lut_0[3][0];
        hwaCfg->thrx[4u]            = dccExtCfg->dpc_lut_0[4][0];
        hwaCfg->thrx[5u]            = dccExtCfg->dpc_lut_0[5][0];
        hwaCfg->thrx[6u]            = dccExtCfg->dpc_lut_0[6][0];
        hwaCfg->thrx[7u]            = dccExtCfg->dpc_lut_0[7][0];

        /* lut2_threshold[] is synchronous to y of lut1 in VPAC3L */
        hwaCfg->lut2_threshold[0u]  = dccExtCfg->dpc_lut_1[0][1];
        hwaCfg->lut2_threshold[1u]  = dccExtCfg->dpc_lut_1[1][1];
        hwaCfg->lut2_threshold[2u]  = dccExtCfg->dpc_lut_1[2][1];
        hwaCfg->lut2_threshold[3u]  = dccExtCfg->dpc_lut_1[3][1];
        hwaCfg->lut2_threshold[4u]  = dccExtCfg->dpc_lut_1[4][1];
        hwaCfg->lut2_threshold[5u]  = dccExtCfg->dpc_lut_1[5][1];
        hwaCfg->lut2_threshold[6u]  = dccExtCfg->dpc_lut_1[6][1];
        hwaCfg->lut2_threshold[7u]  = dccExtCfg->dpc_lut_1[7][1];  

        /* lut2_slope[] is synchronous to slope of lut1 in VPAC3L */
        hwaCfg->lut2_slope[0u]      = dccExtCfg->dpc_lut_1[0][2];
        hwaCfg->lut2_slope[1u]      = dccExtCfg->dpc_lut_1[1][2];
        hwaCfg->lut2_slope[2u]      = dccExtCfg->dpc_lut_1[2][2];
        hwaCfg->lut2_slope[3u]      = dccExtCfg->dpc_lut_1[3][2];
        hwaCfg->lut2_slope[4u]      = dccExtCfg->dpc_lut_1[4][2];
        hwaCfg->lut2_slope[5u]      = dccExtCfg->dpc_lut_1[5][2];
        hwaCfg->lut2_slope[6u]      = dccExtCfg->dpc_lut_1[6][2];
        hwaCfg->lut2_slope[7u]      = dccExtCfg->dpc_lut_1[7][2];  

        /* lut2_thrx[] is synchronous to x of lut0 in VPAC3L */
        hwaCfg->lut2_thrx[0u]       = dccExtCfg->dpc_lut_1[0][0];
        hwaCfg->lut2_thrx[1u]       = dccExtCfg->dpc_lut_1[1][0];
        hwaCfg->lut2_thrx[2u]       = dccExtCfg->dpc_lut_1[2][0];
        hwaCfg->lut2_thrx[3u]       = dccExtCfg->dpc_lut_1[3][0];
        hwaCfg->lut2_thrx[4u]       = dccExtCfg->dpc_lut_1[4][0];
        hwaCfg->lut2_thrx[5u]       = dccExtCfg->dpc_lut_1[5][0];
        hwaCfg->lut2_thrx[6u]       = dccExtCfg->dpc_lut_1[6][0];
        hwaCfg->lut2_thrx[7u]       = dccExtCfg->dpc_lut_1[7][0];        

        hwaCfg->lut_map             = dccExtCfg->dpc_lut_map; 

#else
        hwaCfg->threshold[0] = dccCfg->thr_0;
        hwaCfg->threshold[1] = dccCfg->thr_512;
        hwaCfg->threshold[2] = dccCfg->thr_1024;
        hwaCfg->threshold[3] = dccCfg->thr_2048;
        hwaCfg->threshold[4] = dccCfg->thr_4096;
        hwaCfg->threshold[5] = dccCfg->thr_8192;
        hwaCfg->threshold[6] = dccCfg->thr_16384;
        hwaCfg->threshold[7] = dccCfg->thr_32768;
        hwaCfg->slope[0] = dccCfg->slp_0;
        hwaCfg->slope[1] = dccCfg->slp_512;
        hwaCfg->slope[2] = dccCfg->slp_1024;
        hwaCfg->slope[3] = dccCfg->slp_2048;
        hwaCfg->slope[4] = dccCfg->slp_4096;
        hwaCfg->slope[5] = dccCfg->slp_8192;
        hwaCfg->slope[6] = dccCfg->slp_16384;
        hwaCfg->slope[7] = dccCfg->slp_32768;
#endif

        vissObj->vissCfgRef.dpcOtf = hwaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapCCMParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index)
{
    uint32_t            cnt1, cnt2;
    Fcp_CcmConfig      *ccmCfg = NULL;
    ccmCfg = &vissObj->vissCfg.fcpCfg[fcp_index].ccmCfg;

    if ( (vissObj->dcc_out_prms.useCcmCfg != 0) && (fcp_index == 0) )
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

            vissObj->vissCfgRef.fcpCfg[fcp_index].ccm = ccmCfg;

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
#if defined(VPAC3) /* VPAC3 CC for MV */
    else if ( (vissObj->dcc_out_prms.useVissCcMvCfg != 0) && (fcp_index == 1) )
    {
        for (cnt1 = 0u; cnt1 < FCP_MAX_CCM_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_CCM_COEFF_IN_RAW; cnt2 ++)
            {
                ccmCfg->weights[cnt1][cnt2] = vissObj->dcc_out_prms.vissCcMvCfg.ccm1[cnt1][cnt2];
            }
            ccmCfg->offsets[cnt1] = vissObj->dcc_out_prms.vissCcMvCfg.ccm1[cnt1][FCP_MAX_CCM_COEFF_IN_RAW];
        }

        Fcp_Rgb2YuvConfig *r2y = &vissObj->vissCfg.fcpCfg[fcp_index].rgb2yuvCfg;
        for (cnt1 = 0u; cnt1 < FCP_MAX_RGB2YUV_COEFF; cnt1 ++)
        {
            for (cnt2 = 0u; cnt2 < FCP_MAX_RGB2YUV_COEFF; cnt2 ++)
            {
                r2y->weights[cnt1][cnt2] = vissObj->dcc_out_prms.vissCcMvCfg.rgb2yuv[cnt1][cnt2];
            }
            r2y->offsets[cnt1] = vissObj->dcc_out_prms.vissCcMvCfg.rgb2yuv[cnt1][FCP_MAX_RGB2YUV_COEFF];
        }

        Fcp_GammaConfig *gamma = &vissObj->vissCfg.fcpCfg[fcp_index].gammaCfg;
        gamma->enable = vissObj->dcc_out_prms.vissCcMvCfg.contrast_en;
        gamma->outClip = vissObj->dcc_out_prms.vissCcMvCfg.contrast_clip;

        vissObj->vissCfgRef.fcpCfg[fcp_index].ccm = ccmCfg;
        vissObj->vissCfgRef.fcpCfg[fcp_index].rgb2yuv = r2y;
        vissObj->vissCfgRef.fcpCfg[fcp_index].gamma = gamma;

        /* Setting config flag to 1,
            * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
#endif
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
        vissObj->vissCfgRef.fcpCfg[fcp_index].ccm = ccmCfg;

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

static void tivxVpacVissDccMapFlexCFAParams(tivxVpacVissObj *vissObj, uint32_t fcp_index)
{
    uint32_t cnt;
    Fcp_CfaConfig *cfaCfg;
    viss_ipipe_cfa_flxd   * dcc_cfa_cfg = NULL;
#if defined(VPAC3) || defined(VPAC3L) 
    viss_cfai3_dcc_ext    * dcc_cfai3_ext = NULL;
#endif

    if (NULL != vissObj)
    {
        cfaCfg = &vissObj->vissCfg.fcpCfg[fcp_index].cfaCfg;
#if defined(VPAC3)
        if ((0 == fcp_index) && vissObj->dcc_out_prms.useVissCfai3aCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCfai3aCfg.cfg_cfai1);
            dcc_cfai3_ext = &(vissObj->dcc_out_prms.vissCfai3aCfg.cfg_cfai3);
        }
        else if ((1 == fcp_index) && vissObj->dcc_out_prms.useVissCfai3bCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCfai3bCfg.cfg_cfai1);
            dcc_cfai3_ext = &(vissObj->dcc_out_prms.vissCfai3bCfg.cfg_cfai3);
        }
        else if (vissObj->dcc_out_prms.useCfaCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCFACfg);
        }
#elif defined(VPAC3L)
        if ((0 == fcp_index) && vissObj->dcc_out_prms.useVissCfai3aCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCfai3aCfg.cfg_cfai1);
            dcc_cfai3_ext = &(vissObj->dcc_out_prms.vissCfai3aCfg.cfg_cfai3);
        }
        else if (vissObj->dcc_out_prms.useCfaCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCFACfg);
        }
#else /* VPAC1 */
        if (vissObj->dcc_out_prms.useCfaCfg)
        {
            dcc_cfa_cfg = &(vissObj->dcc_out_prms.vissCFACfg);
        }
#endif
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
            Vhwa_LutConfig *lut16to12Cfg = &vissObj->vissCfg.fcpCfg[fcp_index].cfaLut16to12Cfg;
            lut16to12Cfg->enable    = dcc_cfa_cfg->lut_enable;
            lut16to12Cfg->inputBits = dcc_cfa_cfg->bitWidth;
            lut16to12Cfg->tableAddr = vissObj->dcc_table_ptr.cfa_lut_16to12;

            memcpy(lut16to12Cfg->tableAddr, dcc_cfa_cfg->ToneLut, sizeof(uint32_t) * (uint32_t)FLXD_LUT_SIZE);
            vissObj->vissCfgRef.fcpCfg[fcp_index].cfaLut16to12Cfg = lut16to12Cfg;
        }

#if defined(VPAC3) || defined(VPAC3L)  /* CFAI3 */
        if (NULL != dcc_cfai3_ext)
        {
            Fcp_comDecomLutConfig *compLutCfg = &vissObj->vissCfg.fcpCfg[fcp_index].comLutCfg;
            Fcp_comDecomLutConfig *dcmpLutCfg = &vissObj->vissCfg.fcpCfg[fcp_index].decomLutCfg;

            int k;

            cfaCfg->enable16BitMode = dcc_cfai3_ext->process_mode;
            if (cfaCfg->enable16BitMode)
            {
                cfaCfg->linearBitWidth = dcc_cfai3_ext->dcomp_lut_bw;
                cfaCfg->ccmEnable = dcc_cfai3_ext->ccm_en;
                dcmpLutCfg->enable = dcc_cfai3_ext->dcomp_lut_en;
                compLutCfg->enable = dcc_cfai3_ext->comp_lut_en;

                for (k = 0; k < FCP_MAX_COLOR_COMP; k++)
                {
                    cfaCfg->firConfig[k].enable = 1;
                    cfaCfg->firConfig[k].scaler = dcc_cfai3_ext->out_scaler[k];
                    cfaCfg->firConfig[k].offset = dcc_cfai3_ext->out_offset[k];

                    if (cfaCfg->ccmEnable)
                    {
                        cfaCfg->ccmConfig[k].inputCh0 = dcc_cfai3_ext->ccm[k][0];
                        cfaCfg->ccmConfig[k].inputCh1 = dcc_cfai3_ext->ccm[k][1];
                        cfaCfg->ccmConfig[k].inputCh2 = dcc_cfai3_ext->ccm[k][2];
                        cfaCfg->ccmConfig[k].inputCh3 = dcc_cfai3_ext->ccm[k][3];
                        cfaCfg->ccmConfig[k].offset = dcc_cfai3_ext->ccm[k][4];
                    }

                    if (dcmpLutCfg->enable)
                    {
                        dcmpLutCfg->tableAddr[k] = &vissObj->dcc_table_ptr.dcmpLut[fcp_index][k][0];
                    }
                    if (compLutCfg->enable)
                    {
                        compLutCfg->tableAddr[k] = &vissObj->dcc_table_ptr.compLut[fcp_index][k][0];
                    }
                }

                if (dcmpLutCfg->enable)
                {
                    memcpy(dcmpLutCfg->tableAddr[0], dcc_cfai3_ext->ccmlut_dcmpd_0, CFAI3_DCMPD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(dcmpLutCfg->tableAddr[1], dcc_cfai3_ext->ccmlut_dcmpd_1, CFAI3_DCMPD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(dcmpLutCfg->tableAddr[2], dcc_cfai3_ext->ccmlut_dcmpd_2, CFAI3_DCMPD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(dcmpLutCfg->tableAddr[3], dcc_cfai3_ext->ccmlut_dcmpd_3, CFAI3_DCMPD_LUT_SIZE * sizeof(uint32_t));
                    vissObj->vissCfgRef.fcpCfg[fcp_index].decomLutCfg = dcmpLutCfg;
                }

                if (compLutCfg->enable)
                {
                    memcpy(compLutCfg->tableAddr[0], dcc_cfai3_ext->ccmlut_compd_0, FLXD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(compLutCfg->tableAddr[1], dcc_cfai3_ext->ccmlut_compd_1, FLXD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(compLutCfg->tableAddr[2], dcc_cfai3_ext->ccmlut_compd_2, FLXD_LUT_SIZE * sizeof(uint32_t));
                    memcpy(compLutCfg->tableAddr[3], dcc_cfai3_ext->ccmlut_compd_3, FLXD_LUT_SIZE * sizeof(uint32_t));
                    vissObj->vissCfgRef.fcpCfg[fcp_index].comLutCfg = compLutCfg;
                }
            }
        }
#endif
        vissObj->vissCfgRef.fcpCfg[fcp_index].cfaCfg = cfaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}

static void tivxVpacVissDccMapFlexCCParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res, uint32_t fcp_index)
{
    if (NULL != vissObj)
    {
        tivxVpacVissDccMapCCMParams(vissObj, ae_awb_res, fcp_index);
    }
}

static void tivxVpacVissDccMapFcpParams(tivxVpacVissObj *vissObj,
     tivx_ae_awb_params_t *ae_awb_res)
{
    uint32_t fcp_index;

    for(fcp_index=0; fcp_index < TIVX_VPAC_VISS_FCP_NUM_INSTANCES; fcp_index++)
    {
        tivxVpacVissDccMapYeeParams(vissObj, ae_awb_res, fcp_index);
        tivxVpacVissDccMapFlexCFAParams(vissObj, fcp_index);
        tivxVpacVissDccMapFlexCCParams(vissObj, ae_awb_res, fcp_index);
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
#if defined(VPAC3L)
            viss_lsc_ext_dcc_cfg_t *dcc_ext_cfg = &vissObj->dcc_out_prms.vissLscExtCfg;
#endif
            int32_t len = dcc_cfg->lsc_params.lut_size_in_bytes;

            lscCfg->numTblEntry  = len / 4;
            lscCfg->enable       = dcc_cfg->lsc_params.enable;
            lscCfg->gainFmt      = dcc_cfg->lsc_params.gain_mode_format;
            lscCfg->horzDsFactor = dcc_cfg->lsc_params.gain_mode_m;
            lscCfg->vertDsFactor = dcc_cfg->lsc_params.gain_mode_n;

#if defined(VPAC3L)
            lscCfg->chn_mode     = dcc_ext_cfg->lsc_cfg_mode; 
            for(int i=0; i<RFE_LSC_LUT_MAP_SIZE; i++)
            {
                lscCfg->lut_map[i] = (uint32_t)dcc_ext_cfg->lsc_ch2lut_map[i];
            }
#endif
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

#if defined(VPAC3) || defined(VPAC3L)
static void tivxVpacVissDccMapCacParams(tivxVpacVissObj *vissObj)
{
    if ((NULL != vissObj) && (1 == vissObj->dcc_out_prms.useVissCacCfg))
    {
        viss_cac_dcc_cfg_t * dccCfg = &vissObj->dcc_out_prms.vissCacCfg;
        Cac_Config *hwaCfg = &vissObj->vissCfg.cacCfg;
        hwaCfg->colorEnable = dccCfg->color_en;
        hwaCfg->blkSize = dccCfg->block_s;
        hwaCfg->blkGridSize.hCnt = dccCfg->grid_w;
        hwaCfg->blkGridSize.vCnt = dccCfg->grid_h;
        hwaCfg->displacementLut = vissObj->dcc_table_ptr.cac_lut;

        if(dccCfg->lut_size_in_bytes > CAC_LUT_SIZE)
        {
            VX_PRINT(VX_ZONE_ERROR, "CAC table length is %d bytes, which is greater than CAC_LUT_SIZE (%d bytes)!!!\n", dccCfg->lut_size_in_bytes, CAC_LUT_SIZE);
        }
        memcpy(hwaCfg->displacementLut, dccCfg->cac_lut, dccCfg->lut_size_in_bytes);

        vissObj->vissCfgRef.cacCfg = hwaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}
#endif

#if defined(VPAC3L)
static void tivxVpacVissDccMapPcidParams(tivxVpacVissObj *vissObj)
{
    if ((NULL != vissObj) && (1 == vissObj->dcc_out_prms.useVissPcidCfg))
    {
        tivx_vpac_viss_params_t   *vissPrms = (tivx_vpac_viss_params_t *)
                    vissObj->viss_prms_target_ptr;;

        viss_pcid_dcc_cfg_t * dccCfg = &vissObj->dcc_out_prms.vissPcidCfg;
        Pcid_Cfg *hwaCfg = &vissObj->vissCfg.pcidCfg;

        hwaCfg->cfaFormat = dccCfg->pcid_i_fmt;

        hwaCfg->opChCfg.irOutEn = vissPrms->enable_ir_op;
        hwaCfg->opChCfg.bayerOutEn = vissPrms->enable_bayer_op;
        hwaCfg->opChCfg.bayerOutSel = PCID_BAYEROUTSEL_IR_SUB_BAYER;
        hwaCfg->opChCfg.rbIntpAtIR = dccCfg->pcid_o_fmt;
        hwaCfg->opChCfg.rbIRIntpMethod = PCID_COLOR_INTERPOLATE_HUE;
        hwaCfg->opChCfg.irSubtractEn = TRUE;

        hwaCfg->thRBIrCfg.t1 = dccCfg->pcid_ha_th1;
        hwaCfg->thRBIrCfg.t2 = dccCfg->pcid_ha_th2;
        hwaCfg->thRBIrCfg.t3 = dccCfg->pcid_ha_th3;

        hwaCfg->clrDiffRBIrCfg.gHFXferFactorIr = dccCfg->pcid_hfx_scale_ir;
        hwaCfg->clrDiffRBIrCfg.gHFXferFactor = dccCfg->pcid_hfx_scale;

        hwaCfg->irSubCfg.irSubtractFiltEn = TRUE;
        hwaCfg->irSubCfg.irRemapLutEn = dccCfg->pcid_remap_en;
        hwaCfg->irSubCfg.pIRRemapLut = &vissObj->dcc_table_ptr.pcid_ir_remap_lut;

        /* Can't use memcpy since dcc is 16 bits and driver is 32 bits per entry */
        for(int i=0; i<TIVX_DCC_IR_REMAP_LUT_SIZE; i++)
        {
            hwaCfg->irSubCfg.pIRRemapLut->lut[i] = dccCfg->pcid_remap_lutp[i];
        }

        hwaCfg->irSubCfg.cutOffTh = dccCfg->pcid_irsub_cutoff;
        hwaCfg->irSubCfg.transitionRange = dccCfg->pcid_irsub_trans_bw;
        hwaCfg->irSubCfg.transitionRangeInv = dccCfg->pcid_irsub_trans_bw_recip;
        hwaCfg->irSubCfg.irSubDistScaleLut[0U] = dccCfg->pcid_dist_factor[0];
        hwaCfg->irSubCfg.irSubDistScaleLut[1U] = dccCfg->pcid_dist_factor[1];
        hwaCfg->irSubCfg.irSubDistScaleLut[2U] = dccCfg->pcid_dist_factor[2];
        hwaCfg->irSubCfg.irSubDistScaleLut[3U] = dccCfg->pcid_dist_factor[3];
        hwaCfg->irSubCfg.irSubDistScaleLut[4U] = dccCfg->pcid_dist_factor[4];
        hwaCfg->irSubCfg.irSubFactScale[0U] = dccCfg->pcid_irsub_scale[0];
        hwaCfg->irSubCfg.irSubFactScale[1U] = dccCfg->pcid_irsub_scale[1];
        hwaCfg->irSubCfg.irSubFactScale[2U] = dccCfg->pcid_irsub_scale[2];
        hwaCfg->irSubCfg.irSubFactScale[3U] = dccCfg->pcid_irsub_scale[3];

        vissObj->vissCfgRef.pcidCfg = hwaCfg;

        /* Setting config flag to 1,
         * assumes caller protects this flag */
        vissObj->isConfigUpdated = 1U;
    }
}
#endif

