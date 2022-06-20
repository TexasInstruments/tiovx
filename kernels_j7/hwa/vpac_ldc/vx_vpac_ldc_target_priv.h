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

#ifndef VX_VPAC_LDC_TARGET_PRIV_H_
#define VX_VPAC_LDC_TARGET_PRIV_H_

#include "TI/tivx.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_ldc_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "idcc.h"

#include "ti/drv/vhwa/include/vhwa_m2mLdc.h"

#include "utils/perf_stats/include/app_perf_stats.h"

#define LDC_REMAP_LUT_DRV_EN (0)

#ifdef __cplusplus
extern "C" {
#endif

/*********************************
 *      VPAC_LDC STRUCTURES
 *********************************/

typedef struct
{
    uint32_t                            isAlloc;
    Vhwa_M2mLdcCreateArgs               createArgs;
    Ldc_RdBwLimitConfig                 bwLimitCfg;
    Ldc_Config                          ldc_cfg;
    Fvid2_Handle                        handle;
    tivx_event                          waitForProcessCmpl;
    Ldc_ErrEventParams                  errEvtPrms;
    Ldc_RdBwLimitConfig                 rdBwLimitCfg;
#if LDC_REMAP_LUT_DRV_EN
    Ldc_RemapLutCfg                     lut_cfg;
#endif
    uint32_t                            num_output;

    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    Fvid2_Frame                         inFrm;
    Fvid2_Frame                         outFrm[LDC_MAX_OUTPUT];
    Fvid2_CbParams                      cbPrms;

    uint32_t                            err_stat;
    uint32_t                            sensor_dcc_id;

    /*! Instance ID of the LDC driver */
    uint32_t                            ldc_drv_inst_id;
    /*! HWA Performance ID */
    app_perf_hwa_id_t                   hwa_perf_id;
} tivxVpacLdcObj;


typedef struct
{
    tivx_mutex      lock;
    tivxVpacLdcObj  ldc_obj[VHWA_M2M_LDC_MAX_HANDLES];
} tivxVpacLdcInstObj;


/*********************************
 *      Function Prototypes
 *********************************/

vx_status tivxVpacLdcSetParamsFromDcc(
    const tivxVpacLdcObj                   *ldc_obj,
    const tivx_obj_desc_user_data_object_t *dcc_buf_desc);

#ifdef __cplusplus
}
#endif

#endif /* VX_VPAC_LDC_TARGET_PRIV_H_ */

