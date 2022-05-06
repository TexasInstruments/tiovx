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
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_sde.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_dmpac_sde_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include <math.h>

#include "ti/drv/vhwa/include/vhwa_m2mSde.h"

#include "utils/perf_stats/include/app_perf_stats.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    tivx_dmpac_sde_params_t             sdeParams;
    Vhwa_M2mSdeCreateArgs               createPrms;
    Vhwa_M2mSdePrms                     sdePrms;
    Fvid2_Handle                        handle;
    tivx_event                          waitForProcessCmpl;
    Sde_ErrEventParams                  errEvtPrms;

    Fvid2_Frame                         inFrm[VHWA_M2M_SDE_MAX_IN_BUFFER];
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;
    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    uint32_t                            csHistogram[128U];

    uint32_t                            err_stat;
} tivxDmpacSdeObj;

typedef struct
{
    tivx_mutex lock;
    tivxDmpacSdeObj sdeObj[VHWA_M2M_SDE_MAX_HANDLES];
} tivxDmpacSdeInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacSdeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxDmpacSdeObj *tivxDmpacSdeAllocObject(
       tivxDmpacSdeInstObj *instObj);
static void tivxDmpacSdeFreeObject(
       tivxDmpacSdeInstObj *instObj,
       tivxDmpacSdeObj *sde_obj);
static void tivxDmpacSdeSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static vx_status tivxDmpacSdeGetErrStatusCmd(
       const tivxDmpacSdeObj *sde_obj,
       tivx_obj_desc_scalar_t *scalar_obj_desc);

int32_t tivxDmpacSdeFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxDmpacSdeErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_dmpac_sde_target_kernel = NULL;

tivxDmpacSdeInstObj gTivxDmpacSdeInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelDmpacSde(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0) || (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_1))
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_SDE, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;

        vx_dmpac_sde_target_kernel = tivxAddTargetKernelByName(
                    TIVX_KERNEL_DMPAC_SDE_NAME,
                    target_name,
                    tivxDmpacSdeProcess,
                    tivxDmpacSdeCreate,
                    tivxDmpacSdeDelete,
                    tivxDmpacSdeControl,
                    NULL);
        if (NULL != vx_dmpac_sde_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxDmpacSdeInstObj.lock);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxDmpacSdeInstObj.sdeObj, 0x0,
                    sizeof(tivxDmpacSdeObj) * VHWA_M2M_SDE_MAX_HANDLES);
            }
        }
        else
        {
            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR, "Failed to Add SDE TargetKernel\n");
            status = (vx_status)VX_FAILURE;
        }
    }
}

void tivxRemoveTargetKernelDmpacSde(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_sde_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_sde_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Sde TargetKernel\n");
    }
    if (NULL != gTivxDmpacSdeInstObj.lock)
    {
        tivxMutexDelete(&gTivxDmpacSdeInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacSdeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          size;
    void                             *confidence_histogram_target_ptr = NULL;
    tivxDmpacSdeObj                  *sde_obj = NULL;
    tivx_obj_desc_image_t            *left_desc;
    tivx_obj_desc_image_t            *right_desc;
    tivx_obj_desc_image_t            *output_desc;
    tivx_obj_desc_distribution_t     *confidence_histogram_desc = NULL;
    Fvid2_FrameList                  *inFrmList;
    Fvid2_FrameList                  *outFrmList;
    uint64_t cur_time;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&sde_obj, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Null Desc\n");
        }
        else if (sizeof(tivxDmpacSdeObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* do nothing */
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        inFrmList                 = &sde_obj->inFrmList;
        outFrmList                = &sde_obj->outFrmList;

        left_desc                 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        right_desc                = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        output_desc               = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];

        if( confidence_histogram_desc != NULL)
        {
            confidence_histogram_target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(confidence_histogram_target_ptr,
                confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        /* Initialize SDE Input Frame List */
        inFrmList->frames[SDE_INPUT_BASE_IMG] =
            &sde_obj->inFrm[SDE_INPUT_BASE_IMG];
        inFrmList->frames[SDE_INPUT_REFERENCE_IMG] =
            &sde_obj->inFrm[SDE_INPUT_REFERENCE_IMG];
        inFrmList->numFrames = 2U;

        sde_obj->inFrm[SDE_INPUT_BASE_IMG].addr[0] = tivxMemShared2PhysPtr(
                left_desc->mem_ptr[0].shared_ptr,
                (int32_t)left_desc->mem_ptr[0].mem_heap_region);
        sde_obj->inFrm[SDE_INPUT_REFERENCE_IMG].addr[0] = tivxMemShared2PhysPtr(
                right_desc->mem_ptr[0].shared_ptr,
                (int32_t)right_desc->mem_ptr[0].mem_heap_region);

        /* Initialize SDE Output Frame List */
        outFrmList->frames[0U] = &sde_obj->outFrm;
        outFrmList->numFrames = 1U;

        sde_obj->outFrm.addr[0] = tivxMemShared2PhysPtr(
                output_desc->mem_ptr[0].shared_ptr,
                (int32_t)output_desc->mem_ptr[0].mem_heap_region);

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit SDE Request*/
        fvid2_status = Fvid2_processRequest(sde_obj->handle, inFrmList,
            outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Wait for Frame Completion */
        tivxEventWait(sde_obj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        fvid2_status = Fvid2_getProcessedRequest(sde_obj->handle,
            inFrmList, outFrmList, 0);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
            status = (vx_status)VX_FAILURE;
        }
        
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Get Histogram */
        if(NULL != confidence_histogram_desc)
        {
            fvid2_status = Fvid2_control(sde_obj->handle,
                VHWA_M2M_IOCTL_SDE_GET_HISTOGRAM,
                (uint32_t *) confidence_histogram_target_ptr, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Histogram Request failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_M2mSdePrms                  *sdePrms = NULL;

        sdePrms = &sde_obj->sdePrms;

        

        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_SDE,
            (uint32_t)cur_time,
            sdePrms->sdeCfg.width*sdePrms->sdeCfg.height /* pixels processed */
            );
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if( confidence_histogram_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(confidence_histogram_target_ptr,
                confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }


    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          i;
    uint32_t                          aligned_width;
    uint32_t                          aligned_height;
    tivxDmpacSdeObj                  *sde_obj = NULL;
    Vhwa_M2mSdePrms                  *sdePrms = NULL;
    tivx_dmpac_sde_params_t          *params = NULL;
    tivx_obj_desc_user_data_object_t *params_array = NULL;
    tivx_obj_desc_image_t            *left_desc;
    tivx_obj_desc_image_t            *right_desc;
    tivx_obj_desc_image_t            *output_desc;
    void                             *params_array_target_ptr = NULL;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        sde_obj = tivxDmpacSdeAllocObject(&gTivxDmpacSdeInstObj);
        if (NULL != sde_obj)
        {
            params_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
            left_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
            right_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
            output_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Sde Bilateral Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_M2mSdeCreateArgsInit(&sde_obj->createPrms);

        status = tivxEventCreate(&sde_obj->waitForProcessCmpl);
        if ((vx_status)VX_SUCCESS == status)
        {
            sde_obj->cbPrms.cbFxn   = tivxDmpacSdeFrameComplCb;
            sde_obj->cbPrms.appData = sde_obj;

            sde_obj->handle = Fvid2_create(FVID2_VHWA_M2M_SDE_DRV_ID,
                VHWA_M2M_SDE_DRV_INST_ID, (void *)&sde_obj->createPrms,
                NULL, &sde_obj->cbPrms);

            if (NULL == sde_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Sde Object\n");

                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
        }
    }

    /* Register Error Callback */
    if ((vx_status)VX_SUCCESS == status)
    {
        sde_obj->errEvtPrms.errEvents = VHWA_SDE_RD_ERR | VHWA_SDE_WR_ERR |
            VHWA_SDE_FOCO0_SL2_WR_ERR | VHWA_SDE_FOCO0_VBUSM_RD_ERR;
        sde_obj->errEvtPrms.cbFxn     = tivxDmpacSdeErrorCb;
        sde_obj->errEvtPrms.appData   = sde_obj;

        fvid2_status = Fvid2_control(sde_obj->handle,
            VHWA_M2M_IOCTL_SDE_REGISTER_ERR_CB, &sde_obj->errEvtPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Register Error Callback\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        sdePrms = &sde_obj->sdePrms;

        params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        params = (tivx_dmpac_sde_params_t *)params_array_target_ptr;

        /* Initialize SDE Config with defaults */
        Sde_ConfigInit(&sdePrms->sdeCfg);

        /* Set SDE Config parameters */
        aligned_width = left_desc->imagepatch_addr[0].dim_x;
        aligned_height = left_desc->imagepatch_addr[0].dim_y;
        if (aligned_width < 128U) {
            aligned_width = 128U;    /* Minimum width = 128 */
        }
        if ((aligned_width & 15U) != 0U) {
            aligned_width += 16U;
            aligned_width &= ~15U;   /* Must be multiple of 16 */
        }
        if (aligned_height < 64U) {
            aligned_height = 64U;    /* Minimum height = 64 */
        }
        if ((aligned_height & 15U)!= 0U) {
            aligned_height += 16U;
            aligned_height &= ~15U;   /* Must be multiple of 16 */
        }
        if (SDE_MAX_IMAGE_WIDTH < aligned_width)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Aligned width should be no greater than %d\n", SDE_MAX_IMAGE_WIDTH);
        }
        if (SDE_MAX_IMAGE_HEIGHT < aligned_height)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter height should be no greater than %d\n", SDE_MAX_IMAGE_HEIGHT);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        sdePrms->sdeCfg.enableSDE = 1U;
        sdePrms->sdeCfg.medianFilter = (uint32_t) (params->median_filter_enable);
        sdePrms->sdeCfg.width = aligned_width;
        sdePrms->sdeCfg.height = aligned_height;
        sdePrms->sdeCfg.minDisparity = (uint32_t) (params->disparity_min);
        sdePrms->sdeCfg.searchRange = (uint32_t) (params->disparity_max);
        sdePrms->sdeCfg.lrThreshold = (uint32_t)  (params->threshold_left_right);
        sdePrms->sdeCfg.enableTextureFilter = (uint32_t) (params->texture_filter_enable);
        sdePrms->sdeCfg.textureFilterThreshold = (uint32_t) (params->threshold_texture);
        sdePrms->sdeCfg.penaltyP1 = (uint32_t) (params->aggregation_penalty_p1);
        sdePrms->sdeCfg.penaltyP2 = (uint32_t) (params->aggregation_penalty_p2);
        for(i = 0U; i < DMPAC_SDE_NUM_SCORE_MAP; i++)
        {
            sdePrms->sdeCfg.confScoreMap[i] = (uint32_t) (params->confidence_score_map[i]);
        }

        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG],
            left_desc);
        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_INPUT_REFERENCE_IMG],
            right_desc);
        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_OUTPUT],
            output_desc);

        sdePrms->focoPrms.shiftM1 = 0u;
        sdePrms->focoPrms.dir = 0u;
        sdePrms->focoPrms.round = 0u;

        if(sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat
            == FVID2_CCSF_BITS8_PACKED)
        {
            sdePrms->focoPrms.shiftM1 = 4u;
        }

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           DMPAC SDE parameters needs to be reconfigured */

        memcpy(&sde_obj->sdeParams, params, sizeof(tivx_dmpac_sde_params_t));

        fvid2_status = Fvid2_control(sde_obj->handle,
            VHWA_M2M_IOCTL_SDE_SET_PARAMS, &sde_obj->sdePrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set parameters request failed\n");
            status = (vx_status)VX_FAILURE;
            if (FVID2_EALLOC == fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Not enough SL2 memory for this configuration\n");
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, sde_obj,
            sizeof(tivxDmpacSdeObj));
    }
    else
    {
        if (NULL != sde_obj)
        {
            if (NULL != sde_obj->handle)
            {
                Fvid2_delete(sde_obj->handle, NULL);
                sde_obj->handle = NULL;
            }

            if (NULL != sde_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&sde_obj->waitForProcessCmpl);
            }

            tivxDmpacSdeFreeObject(&gTivxDmpacSdeInstObj, sde_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxDmpacSdeObj    *sde_obj = NULL;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&sde_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != sde_obj) &&
            (sizeof(tivxDmpacSdeObj) == size))
        {
            if (NULL != sde_obj->handle)
            {
                Fvid2_delete(sde_obj->handle, NULL);
                sde_obj->handle = NULL;
            }

            if (NULL != sde_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&sde_obj->waitForProcessCmpl);
            }

            tivxDmpacSdeFreeObject(&gTivxDmpacSdeInstObj, sde_obj);
        }
    }


    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxDmpacSdeObj                  *sde_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&sde_obj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == sde_obj) ||
        (sizeof(tivxDmpacSdeObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Wrong Size for Sde Obj\n");
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
            case TIVX_DMPAC_SDE_CMD_GET_ERR_STATUS:
            {
                status = tivxDmpacSdeGetErrStatusCmd(sde_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Node Command Id\n");
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

static tivxDmpacSdeObj *tivxDmpacSdeAllocObject(
       tivxDmpacSdeInstObj *instObj)
{
    uint32_t        cnt;
    tivxDmpacSdeObj *sde_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->sdeObj[cnt].isAlloc)
        {
            sde_obj = &instObj->sdeObj[cnt];
            memset(sde_obj, 0x0, sizeof(tivxDmpacSdeObj));
            instObj->sdeObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (sde_obj);
}

static void tivxDmpacSdeFreeObject(tivxDmpacSdeInstObj *instObj,
    tivxDmpacSdeObj *sde_obj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
    {
        if (sde_obj == &instObj->sdeObj[cnt])
        {
            sde_obj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}

static void tivxDmpacSdeSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
{
    if (NULL != img_desc)
    {
        switch (img_desc->format)
        {
            case (vx_df_image)VX_DF_IMAGE_U8:
            case (vx_df_image)VX_DF_IMAGE_NV12:
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
            case (vx_df_image)VX_DF_IMAGE_S16:
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
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
        }

        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
    }
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacSdeGetErrStatusCmd(const tivxDmpacSdeObj *sde_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        scalar_obj_desc->data.u32 = sde_obj->err_stat;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxDmpacSdeFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxDmpacSdeObj *sde_obj = (tivxDmpacSdeObj *)appData;

    if (NULL != sde_obj)
    {
        tivxEventPost(sde_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

void tivxDmpacSdeErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxDmpacSdeObj *sde_obj = (tivxDmpacSdeObj *)appData;

    if (NULL != sde_obj)
    {
        if((errEvents & VHWA_SDE_RD_ERR) != 0U)
        {
            /* SL2 RD Error */
            errEvents = (errEvents & (~VHWA_SDE_RD_ERR));
        }
        else if((errEvents & VHWA_SDE_WR_ERR) != 0U)
        {
            /* SL2 WR Error */
            errEvents = (errEvents & (~VHWA_SDE_WR_ERR));
        }
        else if((errEvents & VHWA_SDE_FOCO0_SL2_WR_ERR) != 0U)
        {
            /* FOCO0 SL2 WR Error */
            errEvents = (errEvents & (~VHWA_SDE_FOCO0_SL2_WR_ERR));
        }
        else if((errEvents & VHWA_SDE_FOCO0_VBUSM_RD_ERR) != 0U)
        {
            /* FOCO0 VBUSM RD Error */
            errEvents = (errEvents & (~VHWA_SDE_FOCO0_VBUSM_RD_ERR));
        }
        else
        {
            /* do nothing */
        }
    }
}
