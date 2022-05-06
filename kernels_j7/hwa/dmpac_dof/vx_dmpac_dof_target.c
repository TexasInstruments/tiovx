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

#include "TI/tivx.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "tivx_hwa_dmpac_dof_priv.h"
#include "ti/drv/vhwa/include/vhwa_m2mDof.h"

#include "utils/perf_stats/include/app_perf_stats.h"

#define TIVX_DMPAC_DOF_FLOW_VEC_QSIZE   (TIVX_DMPAC_DOF_MAX_FLOW_VECTOR_DELAY+1)

typedef struct
{
    uint32_t                            isAlloc;
    uint32_t                            isFirstFrame;
    uint32_t                            total_pyr_lvl;

    uint64_t                            inter_buff1;
    uint64_t                            inter_buff2;
    uint32_t                            inter_buff_size;

    tivx_dmpac_dof_params_t             dofAppPrms;
    tivx_dmpac_dof_sof_params_t         sofAppPrms;

    Vhwa_M2mDofCreateArgs               createArgs;
    Vhwa_M2mDofPrms                     dofPrms;
    Dof_ConfScoreParam                  csPrms;
    Fvid2_Handle                        handle;
    uint32_t                            err_stat;
    tivx_event                          waitForProcessCmpl;
    Dof_ErrEventParams                  errEvtPrms;
    Vhwa_HtsLimiter                     htsBwLimitCfg;

    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    Fvid2_Frame                         inFrm[VHWA_M2M_DOF_MAX_IN_BUFFER];
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;

    /* Number of internal delay slots to use for applying previous flow vector
     * output to temporal predictor. After the create phase, this field will
     * either be 0 or a known validated non-zero value and will be used in
     * conjunction with 'flow_vector_in_desc' parameter, to determine whether
     * to use internal history buffer 'outFlowVecHistory' or not.
     *
     * A history buffer is considered for use if
     *  |outFlowVecWrIdx - outFlowVecRdIdx] >= flowVecIntDelay
     *
     * In practice |outFlowVecWrIdx - outFlowVecRdIdx] == flowVecIntDelay is
     * maintained under steady state conditions.
     */
    uint32_t                            flowVecIntDelay;

    /* Next write location in the history buffer. This is initialized to 0 and
     * should never take over 'outFlowVecRdIdx'. The check for takeover is
     * not checked explicitly.
     */
    int32_t                             outFlowVecWrIdx;

    /* Next read location in the history buffer. This is initialized to 0 and
     * should never takeover 'outFlowVecWrIdx'. The check for takeover is
     * not checked explicitly.
     */
    int32_t                             outFlowVecRdIdx;

    /* Space to hold the past flow vector information. This buffer is used only
     * if 'flowVecIntDelay' is non-zero and temporal prediction is enabled.
     */
    uintptr_t                           outFlowVecHistory[TIVX_DMPAC_DOF_FLOW_VEC_QSIZE];

} tivxDmpacDofObj;


typedef struct
{
    tivx_mutex      lock;
    tivxDmpacDofObj  dofObj[VHWA_M2M_DOF_MAX_HANDLES];
} tivxDmpacDofInstObj;


static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxDmpacDofObj *tivxDmpacDofAllocObject(tivxDmpacDofInstObj *instObj);
static void tivxDmpacDofFreeObject(tivxDmpacDofInstObj *instObj,
                                                tivxDmpacDofObj *dof_obj);

static void tivxDmpacDofSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    const tivx_dmpac_dof_params_t *dofAppPrms, const tivx_dmpac_dof_sof_params_t *sofAppPrms,
    tivx_obj_desc_t *obj_desc[]);
static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                                    const tivx_dmpac_dof_params_t *dofAppPrms, uint32_t output_format);
static vx_status tivxDmpacDofGetErrStatusCmd(const tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                                const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofUpdateSofPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_sof_params_t *sofAppPrms);

int32_t tivxDmpacDofFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxDmpacDofErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_dmpac_dof_target_kernel = NULL;

tivxDmpacDofInstObj gTivxDmpacDofInstObj;

/* Default Confidence score configuration, matching with cmodel */
static const Dof_ConfScoreParam gConfScPrms = {
    224,
    {
        {{7, 6, 0},{0x0000000b, 0x00000010, 0x00000000}, {0x00000395, 0xfffffc6b, 0xfffffc6b, 0xfffffc6b}},
        {{7, 5, 7},{0x0000001b, 0x00000052, 0x00000001}, {0xffffec00, 0x00001400, 0x00001400, 0xffffec00}},
        {{7, 5, 0},{0x0000001b, 0x00000052, 0x00000000}, {0x0000127e, 0xffffed82, 0x0000127e, 0x0000127e}},
        {{0, 5, 5},{0x00000254, 0x000000af, 0x000001ef}, {0xffffff23, 0x000000dd, 0xffffff23, 0x000000dd}},
        {{6, 5, 7},{0x00000003, 0x00000214, 0x0000000b}, {0x000000ab, 0xffffff55, 0xffffff55, 0x000000ab}},
        {{6, 5, 5},{0x0000000a, 0x0000022c, 0x00000052}, {0x000000a6, 0xffffff5a, 0x000000a6, 0xffffff5a}},
        {{6, 7, 6},{0x00000010, 0x0000000b, 0x00000023}, {0xffffff3d, 0x000000c3, 0x000000c3, 0xffffff3d}},
        {{7, 5, 7},{0x00000006, 0x0000023c, 0x0000001b}, {0x00000089, 0xffffff77, 0xffffff77, 0x00000089}},
        {{7, 0, 0},{0x00000011, 0x000002b4, 0x0000053f}, {0x00000072, 0xffffff8e, 0xffffff8e, 0x00000072}},
        {{6, 7, 0},{0x00000010, 0x0000000b, 0x000002e5}, {0xffffff7f, 0x00000081, 0x00000081, 0xffffff7f}},
        {{6, 4, 5},{0x00000003, 0x00001141, 0x0000018f}, {0xffffff85, 0x0000007b, 0xffffff85, 0x0000007b}},
        {{0, 5, 5},{0x000001f4, 0x000001e3, 0x000001bb}, {0x00000082, 0xffffff7e, 0xffffff7e, 0x00000082}},
        {{6, 4, 5},{0x00000003, 0x0000129c, 0x00000123}, {0xffffffb8, 0x00000048, 0xffffffb8, 0x00000048}},
        {{5, 5, 6},{0x000000af, 0x00000012, 0x0000006a}, {0xffffffb5, 0x0000004b, 0xffffffb5, 0x0000004b}},
        {{4, 1, 7},{0x00002e5a, 0x00000001, 0x000000c6}, {0xffffffb3, 0x0000004d, 0xffffffb3, 0x0000004d}},
        {{5, 5, 5},{0x000000a3, 0x000000a3, 0x000002de}, {0xffffc000, 0x00000000, 0x00000000, 0xffffc000}},
    }
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelDmpacDof(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0 || self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_1)
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_DOF, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DMPAC_DOF_NAME,
                            target_name,
                            tivxDmpacDofProcess,
                            tivxDmpacDofCreate,
                            tivxDmpacDofDelete,
                            tivxDmpacDofControl,
                            NULL);
        if (NULL != vx_dmpac_dof_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxDmpacDofInstObj.lock);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxDmpacDofInstObj.dofObj, 0x0,
                    sizeof(tivxDmpacDofObj) * VHWA_M2M_DOF_MAX_HANDLES);
            }
        }
        else
        {
            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR, "Failed to Add DOF TargetKernel\n");
        }
    }
}

void tivxRemoveTargetKernelDmpacDof(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_dof_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Dof TargetKernel\n");
    }
    if (NULL != gTivxDmpacDofInstObj.lock)
    {
        tivxMutexDelete(&gTivxDmpacDofInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    int32_t                       fvid2_status = FVID2_SOK;
    uint32_t                      size;
    uint32_t                      pyr_lvl, pyr_cnt;
    uint32_t                      total_pyr_lvl;
    uint32_t                      isBaseImg;
    uint32_t                      update_prms = 0;
    tivxDmpacDofObj              *dofObj = NULL;
    Fvid2_FrameList              *inFrmList;
    Fvid2_FrameList              *outFrmList;
    tivx_dmpac_dof_params_t      *dofAppPrms = NULL;
    tivx_dmpac_dof_sof_params_t  *sofAppPrms = NULL;
    void                         *target_ptr;

    tivx_obj_desc_user_data_object_t *config_desc;
    tivx_obj_desc_image_t        *input_current_base_desc = NULL;
    tivx_obj_desc_image_t        *input_reference_base_desc = NULL;
    tivx_obj_desc_pyramid_t      *input_current_desc;
    tivx_obj_desc_pyramid_t      *input_reference_desc;
    tivx_obj_desc_image_t        *flow_vector_in_desc = NULL;
    tivx_obj_desc_user_data_object_t *sparse_of_config_desc;
    tivx_obj_desc_image_t        *sparse_of_map_desc = NULL;
    tivx_obj_desc_image_t        *flow_vector_out_desc;
    tivx_obj_desc_distribution_t *confidence_histogram_desc = NULL;
    tivx_obj_desc_image_t    *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t    *img_reference_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    if ((num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&dofObj, &size);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* point to descriptors with correct type */
        input_current_base_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];
        input_reference_base_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_BASE_IDX];
        input_current_desc = (tivx_obj_desc_pyramid_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_reference_desc = (tivx_obj_desc_pyramid_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_config_desc = (tivx_obj_desc_user_data_object_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_CONFIG_IDX];
        sparse_of_map_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];

        tivxGetObjDescList(input_current_desc->obj_desc_id,
                           (tivx_obj_desc_t**)img_current_desc,
                           input_current_desc->num_levels);
        tivxGetObjDescList(input_reference_desc->obj_desc_id,
                           (tivx_obj_desc_t**)img_reference_desc,
                           input_reference_desc->num_levels);


        if(NULL != input_current_base_desc)
        {
            total_pyr_lvl = input_current_desc->num_levels + 1U;
            isBaseImg = 1;
        }
        else
        {
            total_pyr_lvl = input_current_desc->num_levels;
            isBaseImg = 0;
        }

        inFrmList = &dofObj->inFrmList;
        outFrmList = &dofObj->outFrmList;

        /* Initialize DOF Input Frame List */
        inFrmList->frames[DOF_INPUT_REFERENCE_IMG] =
                                &dofObj->inFrm[DOF_INPUT_REFERENCE_IMG];
        inFrmList->frames[DOF_INPUT_CURRENT_IMG] =
                                &dofObj->inFrm[DOF_INPUT_CURRENT_IMG];
        inFrmList->frames[DOF_INPUT_TEMPORAL_PRED] =
                                &dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED];
        inFrmList->frames[DOF_INPUT_PYRAMID_PRED] =
                                &dofObj->inFrm[DOF_INPUT_PYRAMID_PRED];
        inFrmList->frames[DOF_INPUT_SOF] = &dofObj->inFrm[DOF_INPUT_SOF];
        inFrmList->numFrames = 5U;

        /* Initialize DOF Output Frame List */
        outFrmList->frames[0U] = &dofObj->outFrm;
        outFrmList->numFrames = 1U;

        if(total_pyr_lvl != dofObj->total_pyr_lvl)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Pyramid Count\n");
            status = (vx_status)VX_FAILURE;
        }

        if(NULL != sparse_of_config_desc)
        {
            target_ptr = tivxMemShared2TargetPtr(&sparse_of_config_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, sparse_of_config_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            sofAppPrms = (tivx_dmpac_dof_sof_params_t *)target_ptr;

            /* Check if sof configuration changes */
            if(memcmp(sofAppPrms, &dofObj->sofAppPrms,
                sizeof(tivx_dmpac_dof_sof_params_t)) != 0)
            {
                status = tivxDmpacDofUpdateSofPrms(dofObj, sofAppPrms);
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, sparse_of_config_desc->mem_size,
                                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
    }
    else
    {
        status = (vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint64_t cur_time;

        cur_time = tivxPlatformGetTimeInUsecs();

        for(pyr_cnt = total_pyr_lvl; pyr_cnt > 0U; pyr_cnt--)
        {
            pyr_lvl = pyr_cnt - 1U;
            if(0U == isBaseImg)
            {
                dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_reference_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        (int32_t)img_reference_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);

                dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_current_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        (int32_t)img_current_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if(pyr_lvl > 0U)
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_reference_desc[pyr_lvl-1U]->mem_ptr[0].shared_ptr,
                            (int32_t)img_reference_desc[pyr_lvl-1U]->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_current_desc[pyr_lvl-1U]->mem_ptr[0].shared_ptr,
                            (int32_t)img_current_desc[pyr_lvl-1U]->mem_ptr[0].mem_heap_region);
                }
                else
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_reference_base_desc->mem_ptr[0].shared_ptr,
                            (int32_t)input_reference_base_desc->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_current_base_desc->mem_ptr[0].shared_ptr,
                            (int32_t)input_current_base_desc->mem_ptr[0].mem_heap_region);
                }
            }

            dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] = (uint64_t)NULL;
            dofObj->inFrm[DOF_INPUT_SOF].addr[0] = (uint64_t)NULL;

            if(0U == pyr_lvl)
            {
                if(NULL != flow_vector_in_desc)
                {
                    dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_in_desc->mem_ptr[0].shared_ptr,
                            (int32_t)flow_vector_in_desc->mem_ptr[0].mem_heap_region);
                }
                else if (dofObj->flowVecIntDelay != 0)
                {
                    int32_t diff;

                    /* Check if we have enough history to use a past buffer. */
                    diff = dofObj->outFlowVecWrIdx - dofObj->outFlowVecRdIdx;

                    if (diff < 0)
                    {
                        diff += TIVX_DMPAC_DOF_FLOW_VEC_QSIZE;
                    }

                    if (diff >= dofObj->flowVecIntDelay)
                    {
                        dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] =
                            dofObj->outFlowVecHistory[dofObj->outFlowVecRdIdx++];

                        dofObj->outFlowVecRdIdx %= TIVX_DMPAC_DOF_FLOW_VEC_QSIZE;
                    }
                }

                if(NULL != sparse_of_map_desc)
                {
                    dofObj->inFrm[DOF_INPUT_SOF].addr[0] =
                        tivxMemShared2PhysPtr(
                            sparse_of_map_desc->mem_ptr[0].shared_ptr,
                            (int32_t)sparse_of_map_desc->mem_ptr[0].mem_heap_region);
                }
                dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff1;

                dofObj->outFrm.addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_out_desc->mem_ptr[0].shared_ptr,
                            (int32_t)flow_vector_out_desc->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if((pyr_lvl % 2U) == 0U)
                {
                    dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff1;
                    dofObj->outFrm.addr[0] = dofObj->inter_buff2;
                }
                else
                {
                    dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff2;
                    dofObj->outFrm.addr[0] = dofObj->inter_buff1;
                }
            }

            if((vx_status)VX_SUCCESS == status)
            {
                /* Set pyramid level to be processed */
                fvid2_status = Fvid2_control(dofObj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_NEXT_PYR, &pyr_lvl, NULL);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to set next pyramid\n");
                    status = (vx_status)VX_FAILURE;
                }
            }

            if((vx_status)VX_SUCCESS == status)
            {
                /* Submit DOF Request*/
                fvid2_status = Fvid2_processRequest(dofObj->handle, inFrmList,
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
                tivxEventWait(dofObj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

                fvid2_status = Fvid2_getProcessedRequest(dofObj->handle,
                    inFrmList, outFrmList, 0);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                    status = (vx_status)VX_FAILURE;
                }
            }
        }

        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_DOF,
            (uint32_t)cur_time,
            dofObj->dofPrms.coreCfg.width*dofObj->dofPrms.coreCfg.height /* pixels processed */
            );

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Save the out flow vector. */
            if (dofObj->flowVecIntDelay != 0)
            {
                uintptr_t   ptr;

                ptr = tivxMemShared2PhysPtr(
                        flow_vector_out_desc->mem_ptr[0].shared_ptr,
                        (int32_t)flow_vector_out_desc->mem_ptr[0].mem_heap_region);

                dofObj->outFlowVecHistory[dofObj->outFlowVecWrIdx++] = ptr;

                dofObj->outFlowVecWrIdx %= TIVX_DMPAC_DOF_FLOW_VEC_QSIZE;
            }

            /* Get Histogram */
            if(NULL != confidence_histogram_desc)
            {
                target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, confidence_histogram_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

                fvid2_status = Fvid2_control(dofObj->handle,
                                       VHWA_M2M_IOCTL_DOF_GET_HISTOGRAM,
                                       (uint32_t *)target_ptr, NULL);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Histogram Request failed\n");
                    status = (vx_status)VX_FAILURE;
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, confidence_histogram_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
            }

        }

        if(1U == dofObj->isFirstFrame)
        {
            /* This was the first iteration, so the Temporal predictor was
                turned off, Enable the Temporal predictor if configured */
            update_prms = 1;

            dofObj->isFirstFrame = 0u;
        }

        config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

        target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, config_desc->mem_size,
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;

        /* Check if configuration changes */
        if(memcmp(dofAppPrms, &dofObj->dofAppPrms,
            sizeof(tivx_dmpac_dof_params_t)) != 0)
        {
            update_prms = 1;
        }

        if(1U == update_prms)
        {
            status = tivxDmpacDofUpdateCfgPrms(dofObj, dofAppPrms, flow_vector_out_desc->format);
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    tivx_dmpac_dof_params_t          *dofAppPrms = NULL;
    tivx_dmpac_dof_sof_params_t      *sofAppPrms = NULL;
    Vhwa_M2mDofPrms                  *dofPrms = NULL;
    tivxDmpacDofObj                  *dofObj = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *sof_config_desc = NULL;
    tivx_obj_desc_pyramid_t          *input_current_desc;
    tivx_shared_mem_ptr_t            tBuffPtr;
    void                             *target_ptr, *sof_target_ptr;

    if ((num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        dofObj = tivxDmpacDofAllocObject(&gTivxDmpacDofInstObj);
        if (NULL != dofObj)
        {
            /* point to descriptors with correct type */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

            sof_config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_CONFIG_IDX];

            input_current_desc = (tivx_obj_desc_pyramid_t *)
                                obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];

            if(NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX])
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels + 1U;
            }
            else
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels;
            }

            dofObj->isFirstFrame = 1;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of VHWA_M2M_DOF_MAX_HANDLES in pdk/packages/ti/drv/vhwa/include/vhwa_m2mDof.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_m2mDofCreateArgsInit(&dofObj->createArgs);

        status = tivxEventCreate(&dofObj->waitForProcessCmpl);
        if ((vx_status)VX_SUCCESS == status)
        {
            dofObj->cbPrms.cbFxn   = tivxDmpacDofFrameComplCb;
            dofObj->cbPrms.appData = dofObj;

            dofObj->handle = Fvid2_create(FVID2_VHWA_M2M_DOF_DRV_ID,
                VHWA_M2M_DOF_DRV_INST_ID, (void *)&dofObj->createArgs,
                NULL, &dofObj->cbPrms);

            if (NULL == dofObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Handle\n");
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
        dofObj->errEvtPrms.errEvents =
            VHWA_DOF_RD_ERR | VHWA_DOF_WR_ERR |
            VHWA_DOF_MP0_RD_STATUS_ERR | VHWA_DOF_FOCO0_SL2_WR_ERR |
            VHWA_DOF_FOCO0_VBUSM_RD_ERR;
        dofObj->errEvtPrms.cbFxn     = tivxDmpacDofErrorCb;
        dofObj->errEvtPrms.appData   = dofObj;

        fvid2_status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB, &dofObj->errEvtPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Error CB registration failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Set DOF Configuration Parameters */
    if ((vx_status)VX_SUCCESS == status)
    {
        dofPrms = &dofObj->dofPrms;

        target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;

        if(NULL != sof_config_desc)
        {
            sof_target_ptr = tivxMemShared2TargetPtr(&sof_config_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(sof_target_ptr, sof_config_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            sofAppPrms = (tivx_dmpac_dof_sof_params_t *)sof_target_ptr;

            memcpy(&dofObj->sofAppPrms, sofAppPrms, sizeof(tivx_dmpac_dof_sof_params_t));
        }

        tivxDmpacDofSetCfgPrms(dofPrms, dofAppPrms, sofAppPrms, obj_desc);

        if((DOF_PREDICTOR_TEMPORAL == dofAppPrms->base_predictor[0]) ||
           (DOF_PREDICTOR_TEMPORAL == dofAppPrms->base_predictor[1]))
        {
            /* Store the flow vector delay parameter. */
            dofObj->flowVecIntDelay = dofAppPrms->flow_vector_internal_delay_num;
        }
        else
        {
            /* We should ignore the flow_vector_internal_delay_num' if temporal
             * predictor is OFF. Set it to 0 for ease of use in the process
             * function.
             */
            dofObj->flowVecIntDelay = 0;
        }

        if(NULL != sof_config_desc)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(sof_target_ptr, config_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           DOF paramerters needs to be reconfigured */
        memcpy(&dofObj->dofAppPrms, dofAppPrms, sizeof(tivx_dmpac_dof_params_t));

        fvid2_status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dofObj->dofPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set parameters request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    /* Initialize and set the Confidence score parameters to default value */
    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_m2mConfScoreParamInit(&dofObj->csPrms);

        memcpy(&dofObj->csPrms, &gConfScPrms, sizeof(Dof_ConfScoreParam));

        fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS,
                                &dofObj->csPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set CS parameter reqeust failed\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Allocate intermediate buffers for Pyramid processing */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* Size = base image width * height * 2 bytes/pixel / 4 (half scale is largest size needed) */
        dofObj->inter_buff_size = (dofObj->dofPrms.coreCfg.width *
                                   dofObj->dofPrms.coreCfg.height) / 2U;

        status = tivxMemBufferAlloc(&tBuffPtr, dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
        if ((vx_status)VX_SUCCESS == status)
        {
            dofObj->inter_buff1 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                (int32_t)tBuffPtr.mem_heap_region);

            status = tivxMemBufferAlloc(&tBuffPtr, dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Buffer Alloc 1 Failed\n");
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            dofObj->inter_buff2 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                (int32_t)tBuffPtr.mem_heap_region);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Buffer Alloc 2 Failed\n");
        }
    }


    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, dofObj,
            sizeof(tivxDmpacDofObj));
    }
    else
    {
        if (NULL != dofObj)
        {
            if (NULL != dofObj->handle)
            {
                Fvid2_delete(dofObj->handle, NULL);
                dofObj->handle = NULL;
            }

            if (NULL != dofObj->waitForProcessCmpl)
            {
                tivxEventDelete(&dofObj->waitForProcessCmpl);
            }

            tivxDmpacDofFreeObject(&gTivxDmpacDofInstObj, dofObj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxDmpacDofObj *dofObj = NULL;

    if (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&dofObj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != dofObj) &&
            (sizeof(tivxDmpacDofObj) == size))
        {
            if (NULL != dofObj->handle)
            {
                Fvid2_delete(dofObj->handle, NULL);
                dofObj->handle = NULL;
            }

            if (NULL != dofObj->waitForProcessCmpl)
            {
                tivxEventDelete(&dofObj->waitForProcessCmpl);
            }

            if ((int32_t)NULL != (int32_t)dofObj->inter_buff1)
            {
                tivxMemFree( (void*)(uintptr_t)(dofObj->inter_buff1), dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
                dofObj->inter_buff1 = (uint64_t)NULL;
            }

            if ((int32_t)NULL != (int32_t)dofObj->inter_buff2)
            {
                tivxMemFree( (void*)(uintptr_t)(dofObj->inter_buff2), dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
                dofObj->inter_buff2 = (uint64_t)NULL;
            }

            tivxDmpacDofFreeObject(&gTivxDmpacDofInstObj, dofObj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxDmpacDofObj                   *dofObj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&dofObj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == dofObj) ||
        (sizeof(tivxDmpacDofObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
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
            case TIVX_DMPAC_DOF_CMD_CS_PARAMS:
            {
                status = tivxDmpacDofSetCsPrms(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_SET_HTS_BW_LIMIT_PARAMS:
            {
                status = tivxDmpacDofSetHtsBwLimit(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_GET_ERR_STATUS:
            {
                status = tivxDmpacDofGetErrStatusCmd(dofObj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
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

static tivxDmpacDofObj *tivxDmpacDofAllocObject(tivxDmpacDofInstObj *instObj)
{
    uint32_t        cnt;
    tivxDmpacDofObj *dofObj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->dofObj[cnt].isAlloc)
        {
            dofObj = &instObj->dofObj[cnt];
            memset(dofObj, 0x0, sizeof(tivxDmpacDofObj));
            instObj->dofObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (dofObj);
}

static void tivxDmpacDofFreeObject(tivxDmpacDofInstObj *instObj,
    tivxDmpacDofObj *dofObj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
    {
        if (dofObj == &instObj->dofObj[cnt])
        {
            dofObj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}


static void tivxDmpacDofSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
{
    if (NULL != img_desc)
    {
        switch (img_desc->format)
        {
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
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
        }

        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
    }
}

static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    const tivx_dmpac_dof_params_t *dofAppPrms, const tivx_dmpac_dof_sof_params_t *sofAppPrms,
    tivx_obj_desc_t *obj_desc[])
{
    uint32_t                  pyr_cnt;

    tivx_obj_desc_image_t    *input_curr_base_desc = NULL;
    tivx_obj_desc_pyramid_t  *input_curr_desc;
    tivx_obj_desc_image_t    *sof_map_desc = NULL;
    tivx_obj_desc_image_t    *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t    *fv_out_desc;
    Fvid2_Format             *fmt;

    input_curr_desc = (tivx_obj_desc_pyramid_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];

    input_curr_base_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];

    tivxGetObjDescList(input_curr_desc->obj_desc_id,
                       (tivx_obj_desc_t**)img_current_desc,
                       input_curr_desc->num_levels);

    if(NULL != input_curr_base_desc)
    {
        /* Information is in base input */
        dofPrms->tPrmdLvl = input_curr_desc->num_levels + 1U;

        dofPrms->coreCfg.width = input_curr_base_desc->imagepatch_addr[0U].dim_x;
        dofPrms->coreCfg.height = input_curr_base_desc->imagepatch_addr[0U].dim_y;
        dofPrms->flowVectorHeight = input_curr_base_desc->imagepatch_addr[0U].dim_y;

    }
    else
    {
        /* Information is in pyramid input */

        dofPrms->tPrmdLvl = input_curr_desc->num_levels;
        dofPrms->coreCfg.width = img_current_desc[0]->imagepatch_addr[0U].dim_x;
        dofPrms->coreCfg.height = img_current_desc[0]->imagepatch_addr[0U].dim_y;
        dofPrms->flowVectorHeight = img_current_desc[0]->imagepatch_addr[0U].dim_y;
    }

    fv_out_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];

    dofPrms->coreCfg.enableDOF = 1u;
    dofPrms->tPredictor  = DOF_PREDICTOR_DELEY_LEFT;
    dofPrms->mPredictor1 = dofAppPrms->inter_predictor[0];
    dofPrms->mPredictor2 = dofAppPrms->inter_predictor[1];
    dofPrms->bPredictor1 = dofAppPrms->base_predictor[0];
    dofPrms->bPredictor2 = dofAppPrms->base_predictor[1];

    dofPrms->coreCfg.horizontalSearchRange = dofAppPrms->horizontal_search_range;
    dofPrms->coreCfg.topSearchRange = dofAppPrms->vertical_search_range[0];
    dofPrms->coreCfg.bottomSearchRange = dofAppPrms->vertical_search_range[1];
    dofPrms->coreCfg.medianFilter = dofAppPrms->median_filter_enable;

    if(0U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0;
        dofPrms->coreCfg.referenceCensusTransform = 0;
    }
    else if(1U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0;
        dofPrms->coreCfg.referenceCensusTransform = 1;
    }
    else if(2U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 1;
        dofPrms->coreCfg.referenceCensusTransform = 0;
    }
    else
    {
        dofPrms->coreCfg.currentCensusTransform = 1;
        dofPrms->coreCfg.referenceCensusTransform = 1;
    }

    dofPrms->coreCfg.motionSmoothnessFactor = dofAppPrms->motion_smoothness_factor;
    dofPrms->coreCfg.iirFilterAlpha = dofAppPrms->iir_filter_alpha;

    dofPrms->coreCfg.lkConfidanceScore = (fv_out_desc->format == (vx_df_image)VX_DF_IMAGE_U16) ? 0U : 1U;

    if((NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX]) &&
       (NULL != sofAppPrms))
    {
        sof_map_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];

        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] =
                                (uint32_t)sof_map_desc->imagepatch_addr[0].stride_y;

        dofPrms->coreCfg.enableSof   = 1u;
        dofPrms->coreCfg.maxMVsInSof = sofAppPrms->sof_max_pix_in_row;
        dofPrms->flowVectorHeight = sofAppPrms->sof_fv_height;
    }
    else
    {
        dofPrms->coreCfg.enableSof = 0u;
        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] = 0u;
    }

    if((DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1) ||
       (DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2))
    {
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] =
                                    (uint32_t)fv_out_desc->imagepatch_addr[0].stride_y;
        dofPrms->coreCfg.enableSof = 0u;
    }
    else
    {
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0u;
    }

    dofPrms->inOutImgFmt[0][DOF_OUTPUT].pitch[0U] =
                                    (uint32_t)fv_out_desc->imagepatch_addr[0].stride_y;

    for(pyr_cnt = 0; pyr_cnt < dofPrms->tPrmdLvl; pyr_cnt++)
    {
        /* Set for Temporal and SOF image */
        if(0U != pyr_cnt)
        {
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0U;
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_SOF].pitch[0U] = 0U;
        }

        /* Set for reference and current image */
        fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_CURRENT_IMG];

        if(NULL == input_curr_base_desc)
        {
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[pyr_cnt]->imagepatch_addr[0].stride_y;

            if(pyr_cnt > 0U)
            {
                dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    img_current_desc[pyr_cnt]->imagepatch_addr[0].dim_x * 2u;
            }
            if (pyr_cnt < (dofPrms->tPrmdLvl-1u))
            {
                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                    (uint32_t)img_current_desc[pyr_cnt+1u]->imagepatch_addr[0].dim_x * 2u;
            }
        }
        else
        {
            if(pyr_cnt > 0U)
            {
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1U]);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1U]);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[pyr_cnt]->imagepatch_addr[0].dim_x * 2u;

                dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    (uint32_t)img_current_desc[pyr_cnt-1U]->imagepatch_addr[0].dim_x * 2u;
            }
            else
            {
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[0u]->imagepatch_addr[0].dim_x * 2u;
            }
        }

        fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_CURRENT_IMG];

        dofPrms->focoPrms.shiftM1 = 0u;
        dofPrms->focoPrms.dir = 0u;
        dofPrms->focoPrms.round = 0u;

        /* For first frame temporal input is not available */
        if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2)
        {
            dofPrms->bPredictor2 = DOF_PREDICTOR_NONE;
        }
        if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1)
        {
            dofPrms->bPredictor1 = DOF_PREDICTOR_NONE;
        }
    }


    return;
}

static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_params_t *dofAppPrms, uint32_t output_format)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    int32_t                      fvid2_status = FVID2_SOK;

    if(NULL == dofAppPrms)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        dof_obj->dofPrms.mPredictor1 = dofAppPrms->inter_predictor[0];
        dof_obj->dofPrms.mPredictor2 = dofAppPrms->inter_predictor[1];
        dof_obj->dofPrms.bPredictor1 = dofAppPrms->base_predictor[0];
        dof_obj->dofPrms.bPredictor2 = dofAppPrms->base_predictor[1];
        dof_obj->dofPrms.coreCfg.horizontalSearchRange =
                                    dofAppPrms->horizontal_search_range;
        dof_obj->dofPrms.coreCfg.topSearchRange =
                                    dofAppPrms->vertical_search_range[0];
        dof_obj->dofPrms.coreCfg.bottomSearchRange =
                                    dofAppPrms->vertical_search_range[1];
        dof_obj->dofPrms.coreCfg.medianFilter =
                                    dofAppPrms->median_filter_enable;

        if(0U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 0;
        }
        else if(1U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 1;
        }
        else if(2U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 1;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 0;
        }
        else
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 1;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 1;
        }

        dof_obj->dofPrms.coreCfg.motionSmoothnessFactor =
                                    dofAppPrms->motion_smoothness_factor;
        dof_obj->dofPrms.coreCfg.iirFilterAlpha = dofAppPrms->iir_filter_alpha;
        dof_obj->dofPrms.coreCfg.lkConfidanceScore =
                    (output_format == (vx_df_image)VX_DF_IMAGE_U16) ? 0U : 1U;

        fvid2_status = Fvid2_control(dof_obj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dof_obj->dofPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set Parameter request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            memcpy(&dof_obj->dofAppPrms, dofAppPrms,
                                            sizeof(tivx_dmpac_dof_params_t));

            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);

}

static vx_status tivxDmpacDofUpdateSofPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_sof_params_t *sofAppPrms)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    int32_t                      fvid2_status = FVID2_SOK;

    if(NULL == sofAppPrms)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        dof_obj->dofPrms.coreCfg.maxMVsInSof = sofAppPrms->sof_max_pix_in_row;
        dof_obj->dofPrms.flowVectorHeight = sofAppPrms->sof_fv_height;

        fvid2_status = Fvid2_control(dof_obj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dof_obj->dofPrms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set Parameter request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            memcpy(&dof_obj->sofAppPrms, sofAppPrms,
                                            sizeof(tivx_dmpac_dof_sof_params_t));
        }
    }

    return (status);
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    uint32_t                            idx;
    uint32_t                            cnt;
    vx_status                           status = (vx_status)VX_SUCCESS;
    int32_t                             fvid2_status = FVID2_SOK;
    tivx_dmpac_dof_cs_tree_params_t    *cs_prms;
    Dof_ConfScoreParam                  csPrms;
    void                               *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_dmpac_dof_cs_tree_params_t) ==
                usr_data_obj->mem_size)
        {
            cs_prms = (tivx_dmpac_dof_cs_tree_params_t *)target_ptr;

            csPrms.confidanceScoreGain = cs_prms->cs_gain;

            for(idx = 0; idx < DOF_NUM_DECISION_TREES; idx++)
            {
                for(cnt = 0; cnt < 3u; cnt++)
                {
                    csPrms.decisionTree[idx].index[cnt] =
                                    cs_prms->decision_tree_index[idx][cnt];
                    csPrms.decisionTree[idx].threshold[cnt] =
                                    cs_prms->decision_tree_threshold[idx][cnt];
                }
                for(cnt = 0; cnt < 4u; cnt++)
                {
                    csPrms.decisionTree[idx].weight[cnt] =
                                    cs_prms->decision_tree_weight[idx][cnt];
                }
            }

            fvid2_status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS, &csPrms, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Set CS parameter request failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
    }

    return (status);
}

static vx_status tivxDmpacDofGetErrStatusCmd(const tivxDmpacDofObj *dof_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        scalar_obj_desc->data.u32 = dof_obj->err_stat;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                               const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                                status = (vx_status)VX_SUCCESS;
    int32_t                                  fvid2_status = FVID2_SOK;
    Vhwa_HtsLimiter                          hts_limit;
    tivx_dmpac_dof_hts_bw_limit_params_t    *app_hts_prms;
    void                                    *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_dmpac_dof_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_dmpac_dof_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            fvid2_status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_HTS_LIMIT, &hts_limit, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Set HTS limit request failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
    }

    return (status);
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxDmpacDofFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxDmpacDofObj *dof_obj = (tivxDmpacDofObj *)appData;

    if (NULL != dof_obj)
    {
        tivxEventPost(dof_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

void tivxDmpacDofErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxDmpacDofObj *dof_obj = (tivxDmpacDofObj *)appData;

    if (NULL != dof_obj)
    {
        dof_obj->err_stat |= errEvents;
    }
}

