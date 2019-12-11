/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"

#include "ti/drv/vhwa/include/vhwa_m2mLdc.h"
#include "utils/perf_stats/include/app_perf_stats.h"

#include "idcc.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

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

    uint32_t                            num_output;

    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    Fvid2_Frame                         inFrm;
    Fvid2_Frame                         outFrm[LDC_MAX_OUTPUT];
    Fvid2_CbParams                      cbPrms;

    uint32_t                            err_stat;
} tivxVpacLdcObj;


typedef struct
{
    tivx_mutex      lock;
    tivxVpacLdcObj  ldc_obj[VHWA_M2M_LDC_MAX_HANDLES];
} tivxVpacLdcInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxVpacLdcObj *tivxVpacLdcAllocObject(tivxVpacLdcInstObj *instObj);
static void tivxVpacLdcFreeObject(tivxVpacLdcInstObj *instObj,
    tivxVpacLdcObj *ldc_obj);
static void tivxVpacLdcSetRegionParams(Ldc_Config *cfg,
    tivx_obj_desc_user_data_object_t *reg_prms_desc);
static vx_status tivxVpacLdcSetFmt(tivx_vpac_ldc_params_t *ldc_prms,
    Fvid2_Format *fmt, tivx_obj_desc_image_t *img_desc);
static void tivxVpacLdcSetAffineConfig(Ldc_PerspectiveTransformCfg *cfg,
    tivx_obj_desc_matrix_t *warp_matrix_desc);
static vx_status tivxVpacLdcSetMeshParams(Ldc_Config *ldc_cfg,
    tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    tivx_obj_desc_image_t *mesh_img_desc);
static vx_status tivxVpacLdcSetLutParamsCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_lut_t *luma_lut_desc, tivx_obj_desc_lut_t *chroma_lut_desc);
static vx_status tivxVpacLdcGetErrStatusCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacLdcSetRdBwLimitCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);

int32_t tivxVpacLdcFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxVpacLdcErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_ldc_target_kernel = NULL;

tivxVpacLdcInstObj gTivxVpacLdcInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacLdc(void)
{
    vx_status status;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) ||
        (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_LDC1,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_ldc_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
        if (NULL != vx_vpac_ldc_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVpacLdcInstObj.lock);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelVpacLdc: Failed to create Mutex\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                memset(&gTivxVpacLdcInstObj.ldc_obj, 0x0U,
                    sizeof(tivxVpacLdcObj) * VHWA_M2M_LDC_MAX_HANDLES);
            }
        }
        else
        {
            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR,
                "tivxAddTargetKernelVpacLdc: Failed to Add LDC TargetKernel\n");
            status = (vx_status)VX_FAILURE;
        }
    }
}

void tivxRemoveTargetKernelVpacLdc(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_ldc_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_ldc_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxRemoveTargetKernelVpacLdc: Failed to Remove Ldc TargetKernel\n");
        status = (vx_status)VX_FAILURE;
    }

    if (NULL != gTivxVpacLdcInstObj.lock)
    {
        tivxMutexDelete(&gTivxVpacLdcInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status              status = (vx_status)VX_SUCCESS;
    int32_t                fvid2_status = FVID2_SOK;
    uint32_t               size;
    uint32_t               out_cnt;
    uint32_t               plane_cnt;
    Fvid2_Frame           *frm = NULL;
    tivx_obj_desc_image_t *in_frm_desc;
    tivx_obj_desc_image_t *out_frm_desc[2U];
    tivxVpacLdcObj        *ldc_obj = NULL;
    Fvid2_FrameList       *inFrmList;
    Fvid2_FrameList       *outFrmList;
    uint64_t cur_time;

    if ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX]) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcProcess: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&ldc_obj, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcProcess: Failed to get Target Kernel\n");
        }
        else if ((NULL == ldc_obj) ||
            (sizeof(tivxVpacLdcObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcProcess: Invalid Ldc Object\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
        }
        else if ((1u == ldc_obj->ldc_cfg.enableOutput[1U]) &&
                (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX]))
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcProcess: Null Desc for output1\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* do nothing */
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        inFrmList = &ldc_obj->inFrmList;
        outFrmList = &ldc_obj->outFrmList;
        in_frm_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX];
        out_frm_desc[0u] = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
        out_frm_desc[1u] = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];

        inFrmList->frames[0U] = &ldc_obj->inFrm;
        inFrmList->numFrames = 1U;
        outFrmList->frames[0U] = &ldc_obj->outFrm[0U];
        outFrmList->frames[1U] = &ldc_obj->outFrm[1U];
        outFrmList->numFrames = 0U;

        frm = &ldc_obj->inFrm;
        for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
        {
            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
            in_frm_desc->mem_ptr[plane_cnt].shared_ptr,
            in_frm_desc->mem_ptr[plane_cnt].mem_heap_region);
        }

        for (out_cnt = 0u; out_cnt < ldc_obj->num_output; out_cnt ++)
        {
            frm = &ldc_obj->outFrm[out_cnt];
            for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES;
                    plane_cnt ++)
            {
                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                    out_frm_desc[out_cnt]->mem_ptr[plane_cnt].shared_ptr,
                    out_frm_desc[out_cnt]->mem_ptr[plane_cnt].mem_heap_region);
            }
            outFrmList->numFrames ++;
        }

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit LDC Request*/
        fvid2_status = Fvid2_processRequest(ldc_obj->handle, inFrmList,
            outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcProcess: Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Wait for Frame Completion */
        tivxEventWait(ldc_obj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        fvid2_status = Fvid2_getProcessedRequest(ldc_obj->handle,
            inFrmList, outFrmList, 0);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcProcess: Failed to Get Processed Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_LDC,
            cur_time,
            ldc_obj->ldc_cfg.outputFrameWidth*ldc_obj->ldc_cfg.outputFrameHeight /* pixels processed */
            );
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    tivx_vpac_ldc_params_t           *ldc_prms = NULL;
    Ldc_Config                       *ldc_cfg = NULL;
    tivxVpacLdcObj                   *ldc_obj = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_matrix_t           *warp_matrix_desc = NULL;
    tivx_obj_desc_user_data_object_t *reg_prms_desc = NULL;
    tivx_obj_desc_user_data_object_t *mesh_prms_desc = NULL;
    tivx_obj_desc_image_t            *mesh_img_desc = NULL;
    tivx_obj_desc_image_t            *in_img_desc = NULL;
    tivx_obj_desc_image_t            *out0_img_desc = NULL;
    tivx_obj_desc_image_t            *out1_img_desc = NULL;
    void                             *target_ptr;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX]) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcCreate: NULL Params check failed\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        ldc_obj = tivxVpacLdcAllocObject(&gTivxVpacLdcInstObj);
        if (NULL != ldc_obj)
        {
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
            warp_matrix_desc = (tivx_obj_desc_matrix_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
            reg_prms_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PRMS_IDX];
            mesh_prms_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_PRMS_IDX];
            mesh_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_IMG_IDX];
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX];
            out0_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
            out1_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
            dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_DCC_DB_IDX];
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcCreate: Failed to allocate Handle Object, increase VHWA_M2M_LDC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_M2mLdcCreateArgsInit(&ldc_obj->createArgs);

        status = tivxEventCreate(&ldc_obj->waitForProcessCmpl);
        if ((vx_status)VX_SUCCESS == status)
        {
            ldc_obj->cbPrms.cbFxn   = tivxVpacLdcFrameComplCb;
            ldc_obj->cbPrms.appData = ldc_obj;

            ldc_obj->handle = Fvid2_create(FVID2_VHWA_M2M_LDC_DRV_ID,
                VHWA_M2M_LDC_DRV_INST_ID, (void *)&ldc_obj->createArgs,
                NULL, &ldc_obj->cbPrms);

            if (NULL == ldc_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacLdcCreate: Fvid2_create failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcCreate: Failed to allocate Event\n");
        }
    }

    /* Register Error Callback */
    if ((vx_status)VX_SUCCESS == status)
    {
        ldc_obj->errEvtPrms.errEvents =
            VHWA_LDC_PIX_IBLK_OUTOFBOUND_ERR | VHWA_LDC_MESH_IBLK_OUTOFBOUND |
            VHWA_LDC_PIX_IBLK_MEMOVF | VHWA_LDC_MESH_IBLK_MEMOVF |
            VHWA_LDC_IFR_OUTOFBOUND | VHWA_LDC_INT_SZOVF |
            VHWA_LDC_SL2_WR_ERR | VHWA_LDC_VBUSM_RD_ERR;
        ldc_obj->errEvtPrms.cbFxn     = tivxVpacLdcErrorCb;
        ldc_obj->errEvtPrms.appData   = ldc_obj;

        fvid2_status = Fvid2_control(ldc_obj->handle,
            IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB, &ldc_obj->errEvtPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcCreate: Fvid2_control Failed: Register Error Callback\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        ldc_cfg = &ldc_obj->ldc_cfg;

        target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

        tivxMemBufferMap(target_ptr, config_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        ldc_prms = (tivx_vpac_ldc_params_t *)target_ptr;

        /* Initialize LDC Config with defaults */
        ldcCfg_init(ldc_cfg);

        /* Set up input and output image formats */
        ldc_obj->num_output = 1U;
        status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->inFmt, in_img_desc);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if (((vx_df_image)VX_DF_IMAGE_U16 == in_img_desc->format) &&
            (1u == ldc_prms->input_align_12bit))
        {
            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
        }

        ldc_cfg->enableOutput[0U] = (uint32_t)TRUE;
        status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->outFmt[0u], out0_img_desc);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((FVID2_DF_LUMA_ONLY == ldc_cfg->outFmt[0u].dataFormat) ||
            (FVID2_DF_CHROMA_ONLY == ldc_cfg->outFmt[0u].dataFormat))
        {
            ldc_cfg->inFmt.dataFormat = ldc_cfg->outFmt[0u].dataFormat;
        }

        if (NULL != out1_img_desc)
        {
            ldc_cfg->enableOutput[1U] = (uint32_t)TRUE;
            status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->outFmt[1u], out1_img_desc);
            ldc_obj->num_output = 2U;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* By default back mapping is disabled */
        ldc_cfg->enableBackMapping = (uint32_t)FALSE;

        /* Set Luma interpolation type */
        if (0U == ldc_prms->luma_interpolation_type)
        {
            ldc_cfg->lumaIntrType = VHWA_LDC_LUMA_INTRP_BICUBIC;
        }
        else
        {
            ldc_cfg->lumaIntrType = VHWA_LDC_LUMA_INTRP_BILINEAR;
        }
        ldc_cfg->outputStartX = ldc_prms->init_x;
        ldc_cfg->outputStartY = ldc_prms->init_y;

        ldc_cfg->outputFrameWidth = out0_img_desc->imagepatch_addr[0U].dim_x;
        ldc_cfg->outputFrameHeight = out0_img_desc->imagepatch_addr[0U].dim_y;

        if (NULL != dcc_buf_desc)
        {
            dcc_parser_output_params_t *pout = tivxMemAlloc(sizeof(dcc_parser_output_params_t), TIVX_MEM_EXTERNAL);
            if (NULL == pout)
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacLdcCreate: Failed to allocate DCC parser output buffer \n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                vpac_ldc_dcc_cfg_t    *pcfg   = &pout->vpacLdcCfg;
                vpac_ldc_dcc_params_t *params = &pcfg->ldc_dcc_params;

                void *target_ptr_dcc;
                target_ptr_dcc = tivxMemShared2TargetPtr(&dcc_buf_desc->mem_ptr);
                tivxMemBufferMap(target_ptr_dcc, dcc_buf_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                uint8_t * dcc_ldc_buf = (uint8_t *)target_ptr_dcc;
                int dcc_buf_size = dcc_buf_desc->mem_size;

                dcc_parser_input_params_t parser_input = {
                    dcc_ldc_buf,
                    dcc_buf_size,
                    0,
                    0,
                    0,
                    ldc_prms->dcc_camera_id
                };

                pout->useVpacLdcCfg = 0;
                dcc_update(&parser_input, pout);

                if (1 == pout->useVpacLdcCfg)
                {
                    ldc_cfg->perspTrnsformCfg.enableWarp = params->pwarpen;
                    ldc_cfg->perspTrnsformCfg.coeffA     = params->affine_a;
                    ldc_cfg->perspTrnsformCfg.coeffB     = params->affine_b;
                    ldc_cfg->perspTrnsformCfg.coeffC     = params->affine_c;
                    ldc_cfg->perspTrnsformCfg.coeffD     = params->affine_d;
                    ldc_cfg->perspTrnsformCfg.coeffE     = params->affine_e;
                    ldc_cfg->perspTrnsformCfg.coeffF     = params->affine_f;
                    ldc_cfg->perspTrnsformCfg.coeffG     = params->affine_g;
                    ldc_cfg->perspTrnsformCfg.coeffH     = params->affine_h;

                    ldc_cfg->enableMultiRegions          = params->regmode_en;
                    ldc_cfg->outputBlockWidth            = params->ld_obw;
                    ldc_cfg->outputBlockHeight           = params->ld_obh;
                    ldc_cfg->pixelPad                    = params->ld_pad;
                    ldc_cfg->outputStartX                = params->ld_initx;
                    ldc_cfg->outputStartY                = params->ld_inity;

                    if (1 == ldc_cfg->enableMultiRegions)
                    {
                        int cnt1, cnt2;
                        for (cnt1 = 0u; cnt1 < LDC_MAX_HORZ_REGIONS; cnt1 ++)
                        {
                            ldc_cfg->regCfg.width[cnt1] = params->ld_sf_width[cnt1];
                        }

                        for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
                        {
                            ldc_cfg->regCfg.height[cnt1] = params->ld_sf_height[cnt1];
                        }

                        for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
                        {
                            for (cnt2 = 0u; cnt2 < LDC_MAX_HORZ_REGIONS; cnt2 ++)
                            {
                                ldc_cfg->regCfg.enable[cnt1][cnt2]      = params->ld_sf_en[cnt1][cnt2];
                                ldc_cfg->regCfg.blockWidth[cnt1][cnt2]  = params->ld_sf_obw[cnt1][cnt2];
                                ldc_cfg->regCfg.blockHeight[cnt1][cnt2] = params->ld_sf_obh[cnt1][cnt2];
                                ldc_cfg->regCfg.pixelPad[cnt1][cnt2]    = params->ld_sf_pad[cnt1][cnt2];
                            }
                        }
                    }

                    if (0 != params->ldmapen)
                    {
                        Ldc_LutCfg *lut_cfg        = &ldc_cfg->lutCfg;

                        ldc_cfg->enableBackMapping = (uint32_t)TRUE;
                        lut_cfg->width             = params->mesh_frame_w;
                        lut_cfg->height            = params->mesh_frame_h;
                        lut_cfg->dsFactor          = params->table_m;
                        lut_cfg->lineOffset        = params->mesh_table_pitch;

                        //TODO: need to translate to physical address in the future when MMU is on
                        lut_cfg->address = (uint64_t)pcfg->mesh_table;
                    }
                }

                tivxMemBufferUnmap(target_ptr_dcc, dcc_buf_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                tivxMemFree(pout, sizeof(dcc_parser_output_params_t), TIVX_MEM_EXTERNAL);
            }
        }
        else
        {
            tivxVpacLdcSetMeshParams(ldc_cfg, mesh_prms_desc, mesh_img_desc);

            tivxVpacLdcSetRegionParams(ldc_cfg, reg_prms_desc);

            tivxVpacLdcSetAffineConfig(&ldc_cfg->perspTrnsformCfg,
                    warp_matrix_desc);
        }

        fvid2_status = Fvid2_control(ldc_obj->handle,
            IOCTL_VHWA_M2M_LDC_SET_PARAMS, &ldc_obj->ldc_cfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcCreate: Fvid2_control Failed: Set Params \n");
            status = (vx_status)VX_FAILURE;
        }

        tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, ldc_obj,
            sizeof(tivxVpacLdcObj));
    }
    else
    {
        if (NULL != ldc_obj)
        {
            if (NULL != ldc_obj->handle)
            {
                Fvid2_delete(ldc_obj->handle, NULL);
                ldc_obj->handle = NULL;
            }

            if (NULL != ldc_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&ldc_obj->waitForProcessCmpl);
            }

            tivxVpacLdcFreeObject(&gTivxVpacLdcInstObj, ldc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacLdcObj *ldc_obj = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX]) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcDelete: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&ldc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != ldc_obj) &&
            (sizeof(tivxVpacLdcObj) == size))
        {
            if (NULL != ldc_obj->handle)
            {
                Fvid2_delete(ldc_obj->handle, NULL);
                ldc_obj->handle = NULL;
            }

            if (NULL != ldc_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&ldc_obj->waitForProcessCmpl);
            }

            tivxVpacLdcFreeObject(&gTivxVpacLdcInstObj, ldc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacLdcObj                   *ldc_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&ldc_obj, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != ldc_obj) &&
        (sizeof(tivxVpacLdcObj) == size))
    {
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcControl: Failed to get Target Kernel\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS:
            {
                status = tivxVpacLdcSetRdBwLimitCmd(ldc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_SET_LUT_PARAMS:
            {
                status = tivxVpacLdcSetLutParamsCmd(ldc_obj,
                    (tivx_obj_desc_lut_t *)obj_desc[0U],
                    (tivx_obj_desc_lut_t *)obj_desc[1U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacLdcGetErrStatusCmd(ldc_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacLdcControl: Invalid Node Cmd Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacLdcObj *tivxVpacLdcAllocObject(tivxVpacLdcInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacLdcObj *ldc_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->ldc_obj[cnt].isAlloc)
        {
            ldc_obj = &instObj->ldc_obj[cnt];
            memset(ldc_obj, 0x0, sizeof(tivxVpacLdcObj));
            instObj->ldc_obj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (ldc_obj);
}

static void tivxVpacLdcFreeObject(tivxVpacLdcInstObj *instObj,
    tivxVpacLdcObj *ldc_obj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
    {
        if (ldc_obj == &instObj->ldc_obj[cnt])
        {
            ldc_obj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}

static vx_status tivxVpacLdcSetFmt(tivx_vpac_ldc_params_t *ldc_prms,
    Fvid2_Format *fmt, tivx_obj_desc_image_t *img_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != img_desc)
    {
        switch (img_desc->format)
        {
            case (vx_df_image)VX_DF_IMAGE_UYVY:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_UYVY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_YUYV:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_YUYV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_NV12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U8:
            {
                if (0u == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U16:
            {
                if (0u == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            {
                if (0u == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_NV12_P12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            default:
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacLdcSetFmt: Invalid Vx Image Format\n");
                break;
            }
        }

        fmt->width      = img_desc->imagepatch_addr[0].dim_x;
        fmt->height     = img_desc->imagepatch_addr[0].dim_y;

        /*  In case of chroma only mode, LDC assumes chroma is from
         *  YUV420 input and expects the frame size
         *  to be for YUV420 frame, ie frame size of luma, where chroma
         *  height is half of luma height.
         *  Node internally takes care of multiplying height by 2. */
        if (FVID2_DF_CHROMA_ONLY == fmt->dataFormat)
        {
            fmt->height = fmt->height * 2U;
        }

        fmt->pitch[0]   = img_desc->imagepatch_addr[0].stride_y;
        fmt->pitch[1]   = img_desc->imagepatch_addr[1].stride_y;
    }

    return status;
}

static void tivxVpacLdcSetAffineConfig(Ldc_PerspectiveTransformCfg *cfg,
    tivx_obj_desc_matrix_t *warp_matrix_desc)
{
    void *warp_matrix_target_ptr;
    float *mat_addr;

    if (NULL != warp_matrix_desc)
    {
        warp_matrix_target_ptr = tivxMemShared2TargetPtr(&warp_matrix_desc->mem_ptr);

        tivxMemBufferMap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        /* Direct pass to HW registers */
        if (VX_TYPE_INT16 == warp_matrix_desc->data_type)
        {
            int16_t *mat_addr;

            mat_addr = (int16_t *)((uintptr_t)warp_matrix_target_ptr);

            if(3 == warp_matrix_desc->columns)
            {
                cfg->coeffA     = mat_addr[0];
                cfg->coeffB     = mat_addr[3];
                cfg->coeffC     = mat_addr[6];
                cfg->coeffD     = mat_addr[1];
                cfg->coeffE     = mat_addr[4];
                cfg->coeffF     = mat_addr[7];
                cfg->coeffG     = mat_addr[2];
                cfg->coeffH     = mat_addr[5];
                cfg->enableWarp = 1;
            }
            else
            {
                cfg->coeffA     = mat_addr[0];
                cfg->coeffB     = mat_addr[2];
                cfg->coeffC     = mat_addr[4];
                cfg->coeffD     = mat_addr[1];
                cfg->coeffE     = mat_addr[3];
                cfg->coeffF     = mat_addr[5];
                cfg->coeffG     = 0;
                cfg->coeffH     = 0;
                cfg->enableWarp = 0;
            }
        }
        else /* Compute HW registers from floating point warp matrix */
        {
            mat_addr = (float *)((uintptr_t)warp_matrix_target_ptr);

            if(3u == warp_matrix_desc->columns)
            {
				float temp_coeffA = (mat_addr[0] / mat_addr[9]) * 4096.0f;
                cfg->coeffA       = (int16_t)temp_coeffA;
                float temp_coeffB = (mat_addr[3] / mat_addr[9]) * 4096.0f;
                cfg->coeffB       = (int16_t)temp_coeffB;
                float temp_coeffC = (mat_addr[6] / mat_addr[9]) * 8.0f;
                cfg->coeffC       = (int16_t)temp_coeffC;
                float temp_coeffD = (mat_addr[1] / mat_addr[9]) * 4096.0f;
                cfg->coeffD       = (int16_t)temp_coeffD;
                float temp_coeffE = (mat_addr[4] / mat_addr[9]) * 4096.0f;
                cfg->coeffE       = (int16_t)temp_coeffE;
                float temp_coeffF = (mat_addr[7] / mat_addr[9]) * 8.0f;
                cfg->coeffF       = (int16_t)temp_coeffF;
                float temp_coeffG = (mat_addr[2] / mat_addr[9]) * 8388608.0f;
                cfg->coeffG       = (int16_t)temp_coeffG;
                float temp_coeffH = (mat_addr[5] / mat_addr[9]) * 8388608.0f;
                cfg->coeffH       = (int16_t)temp_coeffH;
                cfg->enableWarp   = 1;
            }
            else
            {
				float temp_coeffA = mat_addr[0] * 4096.0f;
                cfg->coeffA       = (int16_t)temp_coeffA;
                float temp_coeffB = mat_addr[2] * 4096.0f;
                cfg->coeffB       = (int16_t)temp_coeffB;
                float temp_coeffC = mat_addr[4] * 8.0f;
                cfg->coeffC       = (int16_t)temp_coeffC;
                float temp_coeffD = mat_addr[1] * 4096.0f;
                cfg->coeffD       = (int16_t)temp_coeffD;
                float temp_coeffE = mat_addr[3] * 4096.0f;
                cfg->coeffE       = (int16_t)temp_coeffE;
                float temp_coeffF = mat_addr[5] * 8.0f;
                cfg->coeffF       = (int16_t)temp_coeffF;
                cfg->coeffG       = 0;
                cfg->coeffH       = 0;
                cfg->enableWarp   = 0;
            }
        }
        tivxMemBufferUnmap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    }
    else
    {
        cfg->coeffA     = 4096U;
        cfg->coeffB     = 0U;
        cfg->coeffC     = 0U;
        cfg->coeffD     = 0U;
        cfg->coeffE     = 4096U;
        cfg->coeffF     = 0U;
        cfg->coeffG     = 0U;
        cfg->coeffH     = 0U;
        cfg->enableWarp = 0U;
    }
}

static vx_status tivxVpacLdcSetMeshParams(Ldc_Config *ldc_cfg,
    tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    tivx_obj_desc_image_t *mesh_img_desc)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    tivx_vpac_ldc_mesh_params_t *mesh_prms = NULL;
    void                        *target_ptr;
    Ldc_LutCfg                  *lut_cfg = NULL;

    if ((NULL != mesh_prms_desc) && (NULL != mesh_img_desc))
    {
        target_ptr = tivxMemShared2TargetPtr(&mesh_prms_desc->mem_ptr);

        tivxMemBufferMap(target_ptr, mesh_prms_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_vpac_ldc_mesh_params_t) == mesh_prms_desc->mem_size)
        {
            mesh_prms = (tivx_vpac_ldc_mesh_params_t *)target_ptr;
            lut_cfg   = &ldc_cfg->lutCfg;

            lut_cfg->address    = tivxMemShared2PhysPtr(mesh_img_desc->mem_ptr[0].shared_ptr,
                mesh_img_desc->mem_ptr[0].mem_heap_region);
            lut_cfg->lineOffset = mesh_img_desc->imagepatch_addr[0].stride_y;
            lut_cfg->dsFactor   = mesh_prms->subsample_factor;
            lut_cfg->width      = mesh_prms->mesh_frame_width;
            lut_cfg->height     = mesh_prms->mesh_frame_height;

            /* Back mapping is enabled, if the mesh params & mesh
             * image are non-null */
            ldc_cfg->enableBackMapping = (uint32_t)TRUE;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcSetMeshParams: Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, mesh_prms_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        /* Disable back mapping if one of the mesh params or mesh
         * image is null */
        ldc_cfg->enableBackMapping = (uint32_t)FALSE;
    }

    return (status);
}

static void tivxVpacLdcSetRegionParams(Ldc_Config *cfg,
    tivx_obj_desc_user_data_object_t *reg_prms_desc)
{
    void                                *target_ptr;
    uint32_t                             cnt1, cnt2;
    tivx_vpac_ldc_region_params_t       *reg_prms = NULL;
    tivx_vpac_ldc_multi_region_params_t *mreg_prms = NULL;

    if (NULL != reg_prms_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&reg_prms_desc->mem_ptr);

        tivxMemBufferMap(target_ptr, reg_prms_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_vpac_ldc_region_params_t) ==
                reg_prms_desc->mem_size)
        {
            reg_prms = (tivx_vpac_ldc_region_params_t*)target_ptr;

            cfg->enableMultiRegions = (uint32_t)FALSE;

            cfg->outputBlockWidth  = reg_prms->out_block_width;
            cfg->outputBlockHeight = reg_prms->out_block_height;
            cfg->pixelPad          = reg_prms->pixel_pad;
        }
        else
        {
            mreg_prms = (tivx_vpac_ldc_multi_region_params_t*)target_ptr;

            cfg->enableMultiRegions = (uint32_t)TRUE;

            for (cnt1 = 0u; cnt1 < LDC_MAX_HORZ_REGIONS; cnt1 ++)
            {
                cfg->regCfg.width[cnt1] = mreg_prms->reg_width[cnt1];
            }

            for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
               cfg->regCfg.height[cnt1] = mreg_prms->reg_height[cnt1];
            }

            for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
                for (cnt2 = 0u; cnt2 < LDC_MAX_HORZ_REGIONS; cnt2 ++)
                {
                    reg_prms = &mreg_prms->reg_params[cnt1][cnt2];

                    cfg->regCfg.enable[cnt1][cnt2] = reg_prms->enable;

                    cfg->regCfg.blockWidth[cnt1][cnt2]  =
                        reg_prms->out_block_width;
                    cfg->regCfg.blockHeight[cnt1][cnt2] =
                        reg_prms->out_block_height;
                    cfg->regCfg.pixelPad[cnt1][cnt2]    = reg_prms->pixel_pad;
                }
            }
        }
    }
    else
    {
        cfg->outputBlockWidth = TIVX_VPAC_LDC_DEF_BLOCK_WIDTH;
        cfg->outputBlockHeight = TIVX_VPAC_LDC_DEF_BLOCK_HEIGHT;
        cfg->pixelPad = TIVX_VPAC_LDC_DEF_PIXEL_PAD;
    }
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */


static vx_status tivxVpacLdcSetRdBwLimitCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_ldc_bandwidth_params_t *bwPrms = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_vpac_ldc_bandwidth_params_t) ==
                usr_data_obj->mem_size)
        {
            bwPrms = (tivx_vpac_ldc_bandwidth_params_t *)target_ptr;

            ldc_obj->rdBwLimitCfg.rdBwLimit = bwPrms->bandwidth_control;
            ldc_obj->rdBwLimitCfg.rdTagCnt = bwPrms->tag_count;
            ldc_obj->rdBwLimitCfg.rdMaxBurstLength =
                bwPrms->max_burst_length;

            status = Fvid2_control(ldc_obj->handle,
                IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT,
                bwPrms, NULL);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacLdcSetRdBwLimit: Failed to set BW Params \n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                status = (vx_status)VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacLdcSetRdBwLimit: Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcSetRdBwLimit: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxVpacLdcSetLutParamsCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_lut_t *luma_lut_desc, tivx_obj_desc_lut_t *chroma_lut_desc)
{
    /* TODO */
    return ((vx_status)VX_SUCCESS);
}

static vx_status tivxVpacLdcGetErrStatusCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        scalar_obj_desc->data.u32 = ldc_obj->err_stat;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcGetErrStatus: Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacLdcFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacLdcObj *ldc_obj = (tivxVpacLdcObj *)appData;

    if (NULL != ldc_obj)
    {
        tivxEventPost(ldc_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

void tivxVpacLdcErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacLdcObj *ldc_obj = (tivxVpacLdcObj *)appData;

    if (NULL != ldc_obj)
    {
        ldc_obj->err_stat |= errEvents;
    }
}
