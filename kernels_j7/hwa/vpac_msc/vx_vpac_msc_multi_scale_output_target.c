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

#include "TI/tivx.h"
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_msc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_msc_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"

#include "ti/drv/vhwa/include/vhwa_m2mMsc.h"

#include "utils/perf_stats/include/app_perf_stats.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct tivxVpacMscScaleInstObj_t tivxVpacMscScaleInstObj;

typedef struct
{
    uint32_t                isAlloc;
    Vhwa_M2mMscCreatePrms   createArgs;
    Vhwa_M2mMscParams       msc_prms;
    Fvid2_Handle            handle;
    tivx_event              wait_for_compl;

    uint32_t                num_output;

    Fvid2_FrameList         inFrmList;
    Fvid2_FrameList         outFrmList;
    Fvid2_Frame             inFrm;
    Fvid2_Frame             outFrm[MSC_MAX_OUTPUT];
    Fvid2_CbParams          cbPrms;

    tivxVpacMscScaleInstObj *inst_obj;

    uint32_t                sc_map_idx[MSC_MAX_OUTPUT];
    Msc_Coeff               coeffCfg;

    uint32_t                num_outputs;
} tivxVpacMscScaleObj;


struct tivxVpacMscScaleInstObj_t
{
    tivx_mutex              lock;
    tivxVpacMscScaleObj     msc_obj[VHWA_M2M_MSC_MAX_HANDLES];

    tivx_target_kernel      target_kernel;

    uint32_t                alloc_sc_fwd_dir;

    uint32_t                msc_drv_inst_id;
} ;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* OPENVX Callback functions */
static vx_status VX_CALLBACK tivxVpacMscScaleProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);
static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);

/* Local Functions */
static tivxVpacMscScaleObj *tivxVpacMscScaleAllocObject(
    tivxVpacMscScaleInstObj *instObj);
static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscScaleObj *msc_obj);
static void tivxVpacMscScaleSetScParams(Msc_ScConfig *sc_cfg,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc);
static void tivxVpacMscScaleSetFmt(Fvid2_Format *fmt,
    tivx_obj_desc_image_t *img_desc);
static void tivxVpacMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    tivx_vpac_msc_output_params_t *out_prms);

/* Driver Callback */
int32_t tivxVpacMscMultiScaleFrameComplCb(Fvid2_Handle handle, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

tivxVpacMscScaleInstObj gTivxVpacMscMScaleInstObj[VHWA_M2M_MSC_MAX_INST];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscMultiScale(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    tivxVpacMscScaleInstObj *inst_obj;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_0) ||
        (self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_1))
    {
        /* Reset all values to 0 */
        memset(gTivxVpacMscMScaleInstObj, 0x0,
            sizeof(tivxVpacMscScaleInstObj) * VHWA_M2M_MSC_MAX_INST);

        for (cnt = 0u; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            inst_obj = &gTivxVpacMscMScaleInstObj[cnt];

            if (0u == cnt)
            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
                    TIVX_TARGET_MAX_NAME);
            }
            else
            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
                    TIVX_TARGET_MAX_NAME);
            }

            inst_obj->target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
                                target_name,
                                tivxVpacMscScaleProcess,
                                tivxVpacMscScaleCreate,
                                tivxVpacMscScaleDelete,
                                tivxVpacMscScaleControl,
                                NULL);
            if (NULL != inst_obj->target_kernel)
            {
                /* Allocate lock mutex */
                status = tivxMutexCreate(&inst_obj->lock);
                if ((vx_status)VX_SUCCESS != status)
                {
                    tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxAddTargetKernelVpacMsc: Failed to create Mutex\n");
                }
                else
                {
                    /* Initialize Instance Object */
                    if (0u == cnt)
                    {
                        inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_0;
                        inst_obj->alloc_sc_fwd_dir = 1U;
                    }
                    else
                    {
                        inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_1;
                        inst_obj->alloc_sc_fwd_dir = 0U;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;

                /* TODO: how to handle this condition */
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelVpacMsc: Failed to Add MSC TargetKernel\n");
            }
        }

        /* Clean up allocated resources */
        if ((vx_status)VX_SUCCESS != status)
        {
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                inst_obj = &gTivxVpacMscMScaleInstObj[cnt];
                if (inst_obj->target_kernel != NULL)
                {
                    tivxRemoveTargetKernel(inst_obj->target_kernel);
                }
                if (inst_obj->lock != NULL)
                {
                    tivxMutexDelete(&inst_obj->lock);
                }
            }
        }
    }
}

void tivxRemoveTargetKernelVpacMscMultiScale(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    tivxVpacMscScaleInstObj *inst_obj;

    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscMScaleInstObj[cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxRemoveTargetKernelVpacMscMultiScale: Failed to Remove Msc TargetKernel\n");
        }

        if (NULL != inst_obj->lock)
        {
            tivxMutexDelete(&inst_obj->lock);
        }
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    vx_uint32                cnt;
    vx_uint32                idx;
    Vhwa_M2mMscParams       *msc_prms = NULL;
    tivxVpacMscScaleObj     *msc_obj = NULL;
    tivx_obj_desc_image_t   *in_img_desc = NULL;
    tivx_obj_desc_image_t   *out_img_desc
        [TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {NULL};
    tivxVpacMscScaleInstObj *inst_obj = NULL;
    tivx_target_kernel       target_kernel = NULL;
    Fvid2_Format            *fmt = NULL;
    Msc_ScConfig            *sc_cfg = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        target_kernel = tivxTargetKernelInstanceGetKernel(kernel);
        if (NULL == target_kernel)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleCreate: Failed to get Target Kernel\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
        }
        else
        {
            inst_obj = NULL;
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                if (target_kernel == gTivxVpacMscMScaleInstObj[cnt].target_kernel)
                {
                    inst_obj = &gTivxVpacMscMScaleInstObj[cnt];
                    break;
                }
            }

            if (NULL == inst_obj)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleCreate: Invalid Target Kernel\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        msc_obj = tivxVpacMscScaleAllocObject(inst_obj);
        if (NULL != msc_obj)
        {
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX];

            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {
                out_img_desc[cnt] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];
            }

            /* Initialize Msc object */
            msc_obj->inst_obj = inst_obj;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleCreate: Failed to allocate Handle Object, increase VHWA_M2M_MSC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            Vhwa_M2mMscCreatePrmsInit(&msc_obj->createArgs);

            status = tivxEventCreate(&msc_obj->wait_for_compl);
            if ((vx_status)VX_SUCCESS == status)
            {
                msc_obj->cbPrms.cbFxn   = tivxVpacMscMultiScaleFrameComplCb;
                msc_obj->cbPrms.appData = msc_obj;

                msc_obj->handle = Fvid2_create(FVID2_VHWA_M2M_MSC_DRV_ID,
                    inst_obj->msc_drv_inst_id, &msc_obj->createArgs,
                    NULL, &msc_obj->cbPrms);

                if (NULL == msc_obj->handle)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscScaleCreate: Fvid2_create failed\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleCreate: Failed to allocate Event\n");
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        msc_prms = &msc_obj->msc_prms;

        Vhwa_m2mMscParamsInit(msc_prms);

        tivxVpacMscScaleSetFmt(&msc_prms->inFmt, in_img_desc);

        if ((vx_df_image)VX_DF_IMAGE_NV12 != out_img_desc[0u]->format)
        {
            /* Only luma only mode, if the output is not NV12,
             * even if the input format is NV12. */
            msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        }

        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            /* Overwrite to multi */
            msc_prms->mscCfg.scCfg[cnt].filtMode = MSC_FILTER_MODE_MULTI_PHASE; //MSC_FILTER_MODE_SINGLE_PHASE;

            if (NULL != out_img_desc[cnt])
            {
                if (1U == inst_obj->alloc_sc_fwd_dir)
                {
                    idx = cnt;
                    fmt = &msc_prms->outFmt[idx];
                    sc_cfg = &msc_prms->mscCfg.scCfg[idx];
                    msc_obj->sc_map_idx[cnt] = idx;
                }
                else
                {
                    idx = MSC_MAX_OUTPUT - 1U - cnt;
                    fmt = &msc_prms->outFmt[idx];
                    sc_cfg = &msc_prms->mscCfg.scCfg[idx];
                    msc_obj->sc_map_idx[cnt] = idx;
                }

                tivxVpacMscScaleSetScParams(sc_cfg, in_img_desc, out_img_desc[cnt]);
                tivxVpacMscScaleSetFmt(fmt, out_img_desc[cnt]);

                float temp_horzAccInit = (((((float)sc_cfg->inRoi.cropWidth/(float)sc_cfg->outWidth) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
                sc_cfg->horzAccInit = (uint32_t)temp_horzAccInit;
                float temp_vertAccInit = (((((float)sc_cfg->inRoi.cropHeight/(float)sc_cfg->outHeight) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
                sc_cfg->vertAccInit = (uint32_t)temp_vertAccInit;
            }
            else
            {
                break;
            }
        }

        msc_obj->num_outputs = cnt;

        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
            msc_prms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleCreate: Fvid2_control Failed: Set Params\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* Set up Frame List */
            msc_obj->inFrmList.frames[0u] = &msc_obj->inFrm;

            for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
            {
                msc_obj->outFrmList.frames[cnt] = &msc_obj->outFrm[cnt];
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, msc_obj,
            sizeof(tivxVpacMscScaleObj));
    }
    else
    {
        if (NULL != msc_obj)
        {
            if (NULL != msc_obj->handle)
            {
                Fvid2_delete(msc_obj->handle, NULL);
                msc_obj->handle = NULL;
            }

            if (NULL != msc_obj->wait_for_compl)
            {
                tivxEventDelete(&msc_obj->wait_for_compl);
            }

            tivxVpacMscScaleFreeObject(inst_obj, msc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacMscScaleObj *msc_obj = NULL;
    tivxVpacMscScaleInstObj *inst_obj = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleDelete: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj) &&
            (sizeof(tivxVpacMscScaleObj) == size))
        {
            inst_obj = msc_obj->inst_obj;

            if (NULL != msc_obj->handle)
            {
                Fvid2_delete(msc_obj->handle, NULL);
                msc_obj->handle = NULL;
            }

            if (NULL != msc_obj->wait_for_compl)
            {
                tivxEventDelete(&msc_obj->wait_for_compl);
            }

            tivxVpacMscScaleFreeObject(inst_obj, msc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    uint32_t                 size;
    uint32_t                 cnt;
    uint32_t                 plane_cnt;
    uint32_t                 idx;
    Fvid2_Frame             *frm = NULL;
    tivx_obj_desc_image_t   *in_img_desc;
    tivx_obj_desc_image_t   *out_img_desc[MSC_MAX_OUTPUT];
    tivxVpacMscScaleObj     *msc_obj = NULL;
    Fvid2_FrameList         *inFrmList;
    Fvid2_FrameList         *outFrmList;
    Msc_ScConfig            *sc_cfg;
    uint64_t                cur_time;
    tivxVpacMscScaleInstObj *inst_obj = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleProcess: NULL Params check failed\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        in_img_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX];
        for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
        {
            out_img_desc[cnt] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj) &&
            (sizeof(tivxVpacMscScaleObj) == size))
        {
            for (cnt = 0u; cnt < msc_obj->num_outputs; cnt ++)
            {
                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                idx = cnt; // + TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX;
                if (((uint32_t)TRUE == sc_cfg->enable) &&
                    (NULL == out_img_desc[idx]))
                {
                    VX_PRINT(VX_ZONE_ERROR,
                       "tivxVpacMscScaleProcess: Null Descriptor for Enabled Optional Output\n");
                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                    break;
                }
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleProcess: Invalid Target Instance Context\n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        inFrmList = &msc_obj->inFrmList;
        outFrmList = &msc_obj->outFrmList;

        inFrmList->numFrames = 1U;

        frm = &msc_obj->inFrm;
        for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
        {
            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                (int32_t)in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
        }

        outFrmList->numFrames = 0u;
        for (cnt = 0u; cnt < msc_obj->num_outputs; cnt ++)
        {
            idx = msc_obj->sc_map_idx[cnt];
            sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];
            frm = &msc_obj->outFrm[idx];

            idx = cnt;// + TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX;
            for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES;
                    plane_cnt ++)
            {
                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                    out_img_desc[idx]->mem_ptr[plane_cnt].shared_ptr,
                    (int32_t)out_img_desc[idx]->mem_ptr[plane_cnt].
                    mem_heap_region);
            }
            outFrmList->numFrames ++;
        }

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit MSC Request*/
        fvid2_status = Fvid2_processRequest(msc_obj->handle, inFrmList,
            outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleProcess: Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Wait for Frame Completion */
        tivxEventWait(msc_obj->wait_for_compl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        fvid2_status = Fvid2_getProcessedRequest(msc_obj->handle,
            inFrmList, outFrmList, 0);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleProcess: Failed to Get Processed Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        inst_obj = msc_obj->inst_obj;

        if (VPAC_MSC_INST_ID_0 == inst_obj->msc_drv_inst_id)
        {
            appPerfStatsHwaUpdateLoad(APP_PERF_HWA_MSC0,
                (uint32_t)cur_time,
                in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y /* pixels processed */
                );
        }
        else
        {
            appPerfStatsHwaUpdateLoad(APP_PERF_HWA_MSC1,
                (uint32_t)cur_time,
                in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y /* pixels processed */
                );
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    uint32_t             size;
    tivxVpacMscScaleObj *msc_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&msc_obj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleControl: Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == msc_obj) ||
        (sizeof(tivxVpacMscScaleObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleControl: Invalid Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_MSC_CMD_SET_COEFF:
            {
                status = tivxVpacMscScaleSetCoeffsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetInputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetOutputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS:
            {
                status = tivxVpacMscScaleSetCropParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleControl: Invalid Command Id\n");
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

static tivxVpacMscScaleObj *tivxVpacMscScaleAllocObject(tivxVpacMscScaleInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacMscScaleObj *msc_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->msc_obj[cnt].isAlloc)
        {
            msc_obj = &instObj->msc_obj[cnt];
            memset(msc_obj, 0x0, sizeof(tivxVpacMscScaleObj));
            instObj->msc_obj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (msc_obj);
}

static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscScaleObj *msc_obj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    {
        if (msc_obj == &instObj->msc_obj[cnt])
        {
            msc_obj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}

static void tivxVpacMscScaleSetFmt(Fvid2_Format *fmt,
    tivx_obj_desc_image_t *img_desc)
{
    if (NULL != img_desc)
    {
        switch (img_desc->format)
        {
            case (vx_df_image)VX_DF_IMAGE_NV12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U8:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U16:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleSetFmt: Invalid Vx Image Format\n");
                break;
            }
        }

        fmt->width      = img_desc->imagepatch_addr[0].dim_x;
        fmt->height     = img_desc->imagepatch_addr[0].dim_y;
        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
        fmt->pitch[1]   = (uint32_t)img_desc->imagepatch_addr[1].stride_y;
    }
}

static void tivxVpacMscScaleSetScParams(Msc_ScConfig *sc_cfg,
    tivx_obj_desc_image_t *in_img_desc,
    tivx_obj_desc_image_t *out_img_desc)
{
    if ((NULL != in_img_desc) && (NULL != out_img_desc))
    {
        sc_cfg->enable = TRUE;
        sc_cfg->outWidth = out_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->outHeight = out_img_desc->imagepatch_addr[0].dim_y;
        sc_cfg->inRoi.cropStartX = 0u;
        sc_cfg->inRoi.cropStartY = 0u;
        sc_cfg->inRoi.cropWidth = in_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y;
    }
}

static void tivxVpacMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    tivx_vpac_msc_output_params_t *out_prms)
{
    sc_cfg->isSignedData = out_prms->signed_data;

    sc_cfg->coeffShift = out_prms->coef_shift;
    sc_cfg->isEnableFiltSatMode = out_prms->saturation_mode;

    sc_cfg->inRoi.cropStartX = out_prms->offset_x;
    sc_cfg->inRoi.cropStartY = out_prms->offset_y;

    /* Single Phase Coefficients */
    if (0u == out_prms->filter_mode)
    {
        sc_cfg->filtMode = MSC_FILTER_MODE_SINGLE_PHASE;

        /* Select one of the dedicated single phase coefficient */
        if (0u == out_prms->single_phase.horz_coef_src)
        {
            if (0u == out_prms->single_phase.horz_coef_sel)
            {
                sc_cfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            }
            else
            {
                sc_cfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_1;
            }
        }
        else /* Select one from multi-phase coefficients */
        {
            /* Value of vert_coef_sel is from 0-31,
             * but driver requires 2 to 17, so adding lowest value here */
            sc_cfg->hsSpCoeffSel = out_prms->single_phase.horz_coef_sel +
                MSC_SINGLE_PHASE_MP_COEFF0_0;
        }

        /* Select one of the dedicated single phase coefficient */
        if (0u == out_prms->single_phase.vert_coef_src)
        {
            if (0u == out_prms->single_phase.vert_coef_sel)
            {
                sc_cfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            }
            else
            {
                sc_cfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_1;
            }
        }
        else /* Select one from multi-phase coefficients */
        {
            /* Value of vert_coef_sel is from 0-31,
             * but driver requires 2 to 17, so adding lowest value here */
            sc_cfg->vsSpCoeffSel = out_prms->single_phase.vert_coef_sel +
                MSC_SINGLE_PHASE_MP_COEFF0_0;
        }
    }
    else /* Multi Phase Coefficients */
    {
        sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;

        /* 64 Phase Coefficients */
        if (0u == out_prms->multi_phase.phase_mode)
        {
            sc_cfg->phaseMode = MSC_PHASE_MODE_64PHASE;

            /* Used Coefficent 0 for horizontal scaling */
            if (0u == out_prms->multi_phase.horz_coef_sel)
            {
                sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            }
            else /* Used Coefficent 1 for horizontal scaling */
            {
                sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_2;
            }

            /* Used Coefficent 0 for vertical scaling */
            if (0u == out_prms->multi_phase.vert_coef_sel)
            {
                sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            }
            else /* Used Coefficent 2 for vertical scaling */
            {
                sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_2;
            }
        }
        else /* 32 Phase Coefficients */
        {
            sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;

            switch (out_prms->multi_phase.horz_coef_sel)
            {
                case 0:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
                case 1:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
                    break;
                case 2:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
                    break;
                case 3:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscScaleCopyOutPrmsToScCfg: Incorrect multi-phase horz coeff, defaulting to set 0\n");
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
            }

            switch (out_prms->multi_phase.vert_coef_sel)
            {
                case 0:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
                case 1:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
                    break;
                case 2:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
                    break;
                case 3:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscScaleCopyOutPrmsToScCfg: Incorrect multi-phase horz coeff, defaulting to set 0\n");
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
            }
        }
    }

    sc_cfg->horzAccInit = out_prms->multi_phase.init_phase_x;
    sc_cfg->vertAccInit = out_prms->multi_phase.init_phase_y;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          cnt;
    tivx_vpac_msc_coefficients_t     *coeffs = NULL;
    void                             *target_ptr;
    Msc_Coeff                        *coeffCfg;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        if (sizeof(tivx_vpac_msc_coefficients_t) ==
                usr_data_obj->mem_size)
        {
            coeffs = (tivx_vpac_msc_coefficients_t *)target_ptr;
            coeffCfg = &msc_obj->coeffCfg;

            Msc_coeffInit(coeffCfg);

            for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
            {
                coeffCfg->spCoeffSet[cnt] = &coeffs->single_phase[cnt][0u];
            }

            for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
            {
                coeffCfg->mpCoeffSet[cnt] = &coeffs->multi_phase[cnt][0u];
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetCoeffsCmd: Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleSetCoeffsCmd: Data Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            coeffCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetCoeffsCmd: Failed to create coefficients\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, idx;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;

    for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);

            tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            if (sizeof(tivx_vpac_msc_output_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                tivxVpacMscScaleCopyOutPrmsToScCfg(sc_cfg, out_prms);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleSetOutputParamsCmd: Invalid Mem Size for Output Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetOutputParamsCmd: Failed to Set Output Params\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        if (sizeof(tivx_vpac_msc_input_params_t) ==
                usr_data_obj->mem_size)
        {
            in_prms = (tivx_vpac_msc_input_params_t *)target_ptr;

            msc_obj->msc_prms.enableLineSkip = in_prms->src_ln_inc_2;

            switch (in_prms->kern_sz)
            {
                case 3:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_3TAPS;
                    break;
                case 4:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_4TAPS;
                    break;
                case 5:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_5TAPS;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscScaleSetInputParamsCmd: Invalid Kernel Size\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
            }

        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetInputParamsCmd: Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleSetInputParamsCmd: User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetInputParamsCmd: Failed to Set Input Params\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = VX_SUCCESS;
    uint32_t                          cnt, idx;
    tivx_vpac_msc_crop_params_t      *out_prms = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;

    for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);

            tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            if (sizeof(tivx_vpac_msc_crop_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_crop_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                sc_cfg->inRoi.cropStartX = out_prms->crop_start_x;
                sc_cfg->inRoi.cropStartY = out_prms->crop_start_y;
                sc_cfg->inRoi.cropWidth = out_prms->crop_width;
                sc_cfg->inRoi.cropHeight = out_prms->crop_height;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleSetCropParamsCmd: Invalid Mem Size for Crop Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        if ((vx_status)FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetCropParamsCmd: Failed to Set Crop Params\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}


/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacMscMultiScaleFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacMscScaleObj *msc_obj = (tivxVpacMscScaleObj *)appData;

    if (NULL != msc_obj)
    {
        tivxEventPost(msc_obj->wait_for_compl);
    }

    return FVID2_SOK;
}


