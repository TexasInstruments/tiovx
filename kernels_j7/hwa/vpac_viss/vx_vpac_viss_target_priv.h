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

#ifndef VX_VPAC_VISS_TARGET_FVID2_PRIV_H_
#define VX_VPAC_VISS_TARGET_FVID2_PRIV_H_

#include "TI/tivx.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_viss.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "idcc.h"

#include "ti/drv/vhwa/include/vhwa_m2mViss.h"

/* Dependency on vision apps, as it uses UDMA utils
 * for GLBCE Ctx Save/Restore */
#include <utils/udma/include/app_udma.h>
#include "utils/perf_stats/include/app_perf_stats.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************
 *      VPAC_VISS STRUCTURES
 *********************************/

typedef struct
{
    /*! FVID2 VISS Driver Config for FCP CCM Module */
    Fcp_CcmConfig                       ccmCfg;
    /*! FVID2 VISS Driver Config for Flex CFA */
    Fcp_CfaConfig                       cfaCfg;
    Vhwa_LutConfig                      cfaLut16to12Cfg;
    Fcp_Rgb2YuvConfig                   rgb2yuvCfg;
    Fcp_Rgb2HsvConfig                   rgb2hsvCfg;
    Fcp_GammaConfig                     gammaCfg;
    Fcp_YuvSatLutConfig                 yuvSatLutCfg;
    Fcp_HistConfig                      histCfg;
    Fcp_EeConfig                        eeCfg;

#if defined (VPAC3) || defined (VPAC3L)
    Fcp_comDecomLutConfig               comLutCfg;
    Fcp_comDecomLutConfig               decomLutCfg;
#endif

} tivxVpacVissFcpConfig;

typedef struct
{
    /*! FVID2 VISS Driver Config for NSF4 Module */
    Nsf4v_Config                        nsf4Cfg;
    /*! FVID2 VISS Driver Config for H3A Module */
    H3a_Config                          h3aCfg;
    /*! FVID2 VISS Driver Config for RAWFE PWL Module */
    Rfe_PwlConfig                       pwlCfg1;
    /*! FVID2 VISS Driver Config for RAWFE PWL Module */
    Rfe_PwlConfig                       pwlCfg2;
    /*! FVID2 VISS Driver Config for RAWFE PWL Module */
    Rfe_PwlConfig                       pwlCfg3;
    Vhwa_LutConfig                      decomp1Cfg;
    Vhwa_LutConfig                      decomp2Cfg;
    Vhwa_LutConfig                      decomp3Cfg;

    Rfe_WdrConfig                       merge1Cfg;
    Rfe_WdrConfig                       merge2Cfg;

    Vhwa_LutConfig                      rfeLut20to16Cfg;

    Rfe_DpcOtfConfig                    dpcOtfCfg;
    Rfe_DpcLutConfig                    dpcLutCfg;
    Rfe_LscConfig                       lscCfg;
    /*! FVID2 VISS Driver Config for White Balance module, after LSC */
    Rfe_GainOfstConfig                  wbCfg;
    /*! FVID2 VISS Driver Config for H3A Input */
    Rfe_H3aInConfig                     h3aInCfg;
    /*! FVID2 VISS Driver Config for H3A LUT */
    Vhwa_LutConfig                      h3aLutCfg;

    Glbce_Config                        glbceCfg;
    Glbce_PerceptConfig                 fwdPrcpCfg;
    Glbce_PerceptConfig                 revPrcpCfg;

#if defined (VPAC3) || defined (VPAC3L)
    Cac_Config                          cacCfg;
#endif

#ifdef VPAC3L
    Pcid_Cfg                            pcidCfg;
#endif

    tivxVpacVissFcpConfig               fcpCfg[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];

} tivxVpacVissConfig;

typedef struct
{
    Vhwa_LutConfig                     *cfaLut16to12Cfg;
    Fcp_CfaConfig                      *cfaCfg;
    Fcp_CcmConfig                      *ccm;
    Fcp_GammaConfig                    *gamma;
    Fcp_Rgb2HsvConfig                  *rgb2Hsv;
    Fcp_Rgb2YuvConfig                  *rgb2yuv;
    Fcp_YuvSatLutConfig                *yuvSatLutCfg;
    Fcp_EeConfig                       *eeCfg;
    Fcp_HistConfig                     *histCfg;

#if defined (VPAC3) || defined (VPAC3L)
    Fcp_comDecomLutConfig              *comLutCfg;
    Fcp_comDecomLutConfig              *decomLutCfg;
#endif

} tivxVpacVissFcpConfigRef;


typedef struct
{
    Rfe_PwlConfig                      *vsPwlCfg;
    Rfe_PwlConfig                      *sPwlCfg;
    Rfe_PwlConfig                      *lPwlCfg;

    Vhwa_LutConfig                     *vsLutCfg;
    Vhwa_LutConfig                     *sLutCfg;
    Vhwa_LutConfig                     *lLutCfg;

    Rfe_WdrConfig                      *wdr1Cfg;
    Rfe_WdrConfig                      *wdr2Cfg;
    Vhwa_LutConfig                     *comp20To16LutCfg;

    Rfe_DpcOtfConfig                   *dpcOtf;
    Rfe_DpcLutConfig                   *dpcLut;

    Rfe_LscConfig                      *lscCfg;

    Rfe_GainOfstConfig                 *wbCfg;

    Rfe_H3aInConfig                    *rfeH3aInCfg;
    Vhwa_LutConfig                     *h3aLutCfg;

    Nsf4v_Config                       *nsf4Cfg;
    Glbce_Config                       *glbceCfg;
    Glbce_PerceptConfig                *fwdPrcpCfg;
    Glbce_PerceptConfig                *revPrcpCfg;

    H3a_Config                         *h3aCfg;

#if defined (VPAC3) || defined (VPAC3L)
    Cac_Config                         *cacCfg;
#endif

#ifdef VPAC3L
    Pcid_Cfg                           *pcidCfg;
#endif

    tivxVpacVissFcpConfigRef           fcpCfg[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];

} tivxVpacVissConfigRef;


/*! Buffers which are set by DCC and the driver doesn't already have space
 * reserved to put it in the context, can reside here.  This way, if there
 * are multiple cameras configured with different tables, there is a instance
 * specific location as part of the instance context where it can reside
 * independently (this may be a waste of memory in the case of homogenous cameras)*/
typedef struct
{
    uint32_t                           h3aLut[RFE_H3A_COMP_LUT_SIZE];
    uint32_t                           cfa_lut_16to12[FLXD_LUT_SIZE];
    uint32_t                           rawfe_pwl_vshort_lut[FLXD_LUT_SIZE];
    uint32_t                           lsc_lut[RFE_LSC_TBL_SIZE];
    int32_t                            ee_lut[FCP_EE_LUT_SIZE];
#if defined(VPAC3) || defined(VPAC3L)
    int32_t                            cac_lut[CAC_LUT_SIZE];
    uint32_t                           raw_hist_lut[NSF4_HISTOGRAM_LUT_SIZE];
    uint32_t                           dcmpLut[TIVX_VPAC_VISS_FCP_NUM_INSTANCES][FCP_MAX_COLOR_COMP][FLXD_LUT_SIZE];
    uint32_t                           compLut[TIVX_VPAC_VISS_FCP_NUM_INSTANCES][FCP_MAX_COLOR_COMP][FLXD_LUT_SIZE];
#endif
#ifdef VPAC3L
    Pcid_IRremapLut                    pcid_ir_remap_lut;
#endif
} tivxVpacVissDccTables;

typedef struct
{
    /*! Flag to indicate if this object is free or not */
    uint32_t                            isAlloc;

    /*! VISS M2M Driver Create Parameter */
    Vhwa_M2mVissCreateArgs              createArgs;
    /*! VISS M2M Driver Handle */
    Fvid2_Handle                        handle;
    /*! Mutex used for waiting for process completion */
    tivx_event                          waitForProcessCmpl;
    /*! Mutex used for waiting for protecting config */
    tivx_mutex                          config_lock;
    /*! VISS M2M Driver Parameters */
    Vhwa_M2mVissParams                  vissPrms;

    /*! VISS M2M Driver Input Frame List, used for providing
     *  an array of input frames */
    Fvid2_FrameList                     inFrmList;
    /*! VISS M2M Driver Output Frame List, used for providing
     *  an array of output frames */
    Fvid2_FrameList                     outFrmList;
    /*! VISS M2M Driver Input Frames */
    Fvid2_Frame                         inFrm[VHWA_M2M_VISS_MAX_INPUTS];
    /*! VISS M2M Driver Output Frames */
    Fvid2_Frame                         outFrm[VHWA_M2M_VISS_MAX_OUTPUTS];
    /*! VISS M2M Driver Callback parameters */
    Fvid2_CbParams                      cbPrms;

    /* DCC Parameters */
    /*! Flag to indicate if DCC is used or not */
    uint32_t                            use_dcc;
    /*! Sensor DCC ID required for DCC parsing */
    uint32_t                            sensor_dcc_id;
    /*! DCC Parsed output
     *  TODO: This instance can be very big, allocate it.
     */
    dcc_parser_output_params_t          dcc_out_prms;
    /*! DCC Output Buffer required for storing config for module
     *  supporting multi-photo space */
    uint8_t                            *dcc_out_buf;
    /*! Memory size required for storing DCC output parameters,
     *  which supports multi-photospace */
    uint32_t                            dcc_out_numbytes;
    /*! Buffers needed for instance specific DCC configured tables*/
    tivxVpacVissDccTables               dcc_table_ptr;

    /*! Number of input buffer pointers */
    uint32_t                            num_in_buf;
    /*! Number of output buffer pointers for each output */
    uint32_t                            num_out_buf_addr[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];

    /*! VISS configuration containing instances of VISS module's configuration,
     *  supported by driver.
     *  This is mainly used to map DCC output configuration to driver's
     *  configuration. */
    tivxVpacVissConfig                  vissCfg;

    /*! Contains pointer to Module's configuration
     *  When app passes DCC database, it is parsed and output is stored in
     *  vissCfg. The pointers in vissCfgRef point to these objects.
     *  vissCfgRef contains pointer to only those modules, for which
     *  config is passed in DCC database.
     *  When the next process API is called, Pointers in vissCfgRef
     *  are checked and if any of them is set to null, module's
     *  configuration is set in the driver and corresponding pointer
     *  is reset to null. */
    tivxVpacVissConfigRef               vissCfgRef;

    /*! Flag to indicate if the pointers in vissCfgRef are updated
     *  and pointing to new config or not.
     *  When process API is called, this is used to know if config
     *  is updated and if driver can be called to update modules config. */
    uint32_t                            isConfigUpdated;

    /*! Keeps track of the last H3A input Source
     *  At the process time, this is used to check if there is any change
     *  in H3A Source, only then, driver is called to change the source.
     */
    uint32_t                            lastH3aInSrc;
    /*! Locally keeps track NSF bypass flag for easy access */
    uint32_t                            bypass_nsf4;

    /* Few temporary flags, used by the code for unmapping buffers
     * Allocated here, so that Api does not have to create local variables.
     */
    /*! VISS Params Target Pointer */
    void                               *viss_prms_target_ptr;
    /*! aewb_result target Pointer */
    void                               *aewb_res_target_ptr;
    /*! H3A Result target Pointer */
    void                               *h3a_out_target_ptr;
    /*! H3A Output size */
    uint32_t                            h3a_output_size;
    /*! H3A AEW config to copy to h3a output for each frame */
    tivx_h3a_aew_config                 aew_config;
    /*! H3A output enabled flag */
    vx_bool                             h3a_out_enabled;
    /*! Identifier for cpu ID that the VISS node is running on.
     *  Currently used only for notifying AEWB node which cpu to send the update to based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                            cpu_id;
    /*! Identifier for camera channel ID.
     *  Currently used only for notifying AEWB node which channel to update based on these results
     *  when ae_awb_result from the graph is NULL. */
    uint32_t                            channel_id;

    /*! Physical address of the context memory. */
    tivx_shared_mem_ptr_t               ctx_mem_ptr;
    /*! GLBCE Statistics information, contains physical address
     *  of the Stats memory and size */
    uint64_t                            ctx_mem_phys_ptr;
    /*! Physical address of the context memory */
    Glbce_StatsInfo                     glbceStatInfo;
    /*! configuration buffer, if configThroughUDMA is true */
    Vhwa_M2mVissConfigAppBuff           configurationBuffer;
    /*! Instance ID of the VISS driver */
    uint32_t                            viss_drv_inst_id;
    /*! HWA Performance ID */
    app_perf_hwa_id_t                   hwa_perf_id;
} tivxVpacVissObj;

typedef struct
{
    /*! Protects allocation of viss objects */
    tivx_mutex       lock;

    /*! VISS objects */
    tivxVpacVissObj  vissObj[VHWA_M2M_VISS_MAX_HANDLES];
} tivxVpacVissInstObj;


/*********************************
 *      Function Prototypes
 *********************************/

void tivxVpacVissDccMapRfeParams(tivxVpacVissObj *vissObj);

vx_status tivxVpacVissSetDefaultParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms, tivx_ae_awb_params_t *ae_awb_res);

vx_status tivxVpacVissSetParamsFromDcc(tivxVpacVissObj *vissObj,
    const tivx_obj_desc_user_data_object_t *dcc_buf_desc,
    const tivx_obj_desc_user_data_object_t *h3a_out_desc,
    tivx_ae_awb_params_t *ae_awb_res);

/*! Initializes DCC parameters to default values */
vx_status tivxVpacVissInitDcc(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms);

/*! Deinitializes DCC parameters */
void tivxVpacVissDeInitDcc(tivxVpacVissObj *vissObj);

/*! Sets entire VISS config in DRV */
vx_status tivxVpacVissSetConfigInDrv(tivxVpacVissObj *vissObj);

/*! This is mainly used to set/change H3a Input source,
 *  Can be used to runtime change h3a input source.
 */
void tivxVpacVissSetH3aSrcParams(tivxVpacVissObj *vissObj,
    const tivx_vpac_viss_params_t *vissPrms);

/*! This is used to update VISS parameters based on AEWB output.
 *  It uses DCC to get the updated parameter values
 */
vx_status tivxVpacVissApplyAEWBParams(tivxVpacVissObj *vissObj,
    tivx_ae_awb_params_t *aewb_result);

/*!
 * This function is used to set configuration buffer in the driver.
 */
vx_status tivxVpacVissSetConfigBuffer(tivxVpacVissObj *vissObj);

/* This function is used to free the config buffer memory */
void tivxVpacVissDeleteConfigBuffer(tivxVpacVissObj *vissObj);

#ifdef __cplusplus
}
#endif

#endif /* VX_VPAC_VISS_TARGET_FVID2_PRIV_H_ */

