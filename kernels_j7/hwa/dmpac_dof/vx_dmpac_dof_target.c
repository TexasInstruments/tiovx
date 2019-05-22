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
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"

#include "ti/drv/vhwa/include/vhwa_m2mDof.h"


typedef struct
{
    uint32_t                            isAlloc;
    uint32_t                            isFirstFrame;
    uint32_t                            total_pyr_lvl;

    uint64_t                            inter_buff1;
    uint64_t                            inter_buff2;

    tivx_dmpac_dof_params_t             dofAppPrms;

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
    tivx_obj_desc_image_t *img_desc);
static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    tivx_dmpac_dof_params_t *dofAppPrms, tivx_obj_desc_t *obj_desc[]);
static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                                    tivx_dmpac_dof_params_t *dofAppPrms);
static vx_status tivxDmpacDofGetErrStatusCmd(tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                                tivx_obj_desc_user_data_object_t *usr_data_obj);

int32_t tivxDmpacDofFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxDmpacDofErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_dmpac_dof_target_kernel = NULL;

tivxDmpacDofInstObj gTivxDmpacDofInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelDmpacDof(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_IPU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_DOF, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelDmpacDof: Invalid CPU ID\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
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
            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelDmpacDof: Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxDmpacDofInstObj.dofObj, 0x0U,
                    sizeof(tivxDmpacDofObj) * VHWA_M2M_DOF_MAX_HANDLES);
            }
        }
        else
        {
            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR,
                "tivxAddTargetKernelDmpacDof: Failed to Add DOF TargetKernel\n");
        }
    }
}

void tivxRemoveTargetKernelDmpacDof(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_dof_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = NULL;
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
    vx_status                     status = VX_SUCCESS;
    uint32_t                      size;
    uint32_t                      pyr_lvl, pyr_cnt;
    uint32_t                      total_pyr_lvl;
    uint32_t                      isBaseImg;
    uint32_t                      update_prms = 0;
    tivxDmpacDofObj              *dofObj = NULL;
    Fvid2_FrameList              *inFrmList;
    Fvid2_FrameList              *outFrmList;
    tivx_dmpac_dof_params_t      *dofAppPrms = NULL;
    void                         *target_ptr;

    tivx_obj_desc_user_data_object_t *config_desc;
    tivx_obj_desc_image_t        *input_current_base_desc = NULL;
    tivx_obj_desc_image_t        *input_reference_base_desc = NULL;
    tivx_obj_desc_pyramid_t      *input_current_desc;
    tivx_obj_desc_pyramid_t      *input_reference_desc;
    tivx_obj_desc_image_t        *flow_vector_in_desc = NULL;
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
        VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofProcess: Required input parameter set to NULL\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&dofObj, &size);
    }

    if (VX_SUCCESS == status)
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
            total_pyr_lvl = input_current_desc->num_levels + 1;
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

            VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofProcess: Invalid Pyramid Count\n");            {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        for(pyr_cnt = total_pyr_lvl; pyr_cnt > 0; pyr_cnt++)
        {
            pyr_lvl = pyr_cnt - 1;
            if(0 == isBaseImg)
            {
                dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_reference_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        img_reference_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);

                dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_current_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        img_current_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if(pyr_lvl > 0)
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_reference_desc[pyr_lvl-1]->mem_ptr[0].shared_ptr,
                            img_reference_desc[pyr_lvl-1]->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_current_desc[pyr_lvl-1]->mem_ptr[0].shared_ptr,
                            img_current_desc[pyr_lvl-1]->mem_ptr[0].mem_heap_region);
                }
                else
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_reference_base_desc->mem_ptr[0].shared_ptr,
                            input_reference_base_desc->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_current_base_desc->mem_ptr[0].shared_ptr,
                            input_current_base_desc->mem_ptr[0].mem_heap_region);
                }
            }

            dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] = NULL;
            dofObj->inFrm[DOF_INPUT_SOF].addr[0] = NULL;

            if(0 == pyr_lvl)
            {
                if(NULL != flow_vector_in_desc)
                {
                    dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_in_desc->mem_ptr[0].shared_ptr,
                            flow_vector_in_desc->mem_ptr[0].mem_heap_region);
                }
                if(NULL != sparse_of_map_desc)
                {
                    dofObj->inFrm[DOF_INPUT_SOF].addr[0] =
                        tivxMemShared2PhysPtr(
                            sparse_of_map_desc->mem_ptr[0].shared_ptr,
                            sparse_of_map_desc->mem_ptr[0].mem_heap_region);
                }
                dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff1;

                dofObj->outFrm.addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_out_desc->mem_ptr[0].shared_ptr,
                            flow_vector_out_desc->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if(pyr_lvl % 2 == 0)
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

            /* Set pyramid level to be processed */
            status = Fvid2_control(dofObj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_NEXT_PYR, &pyr_lvl, NULL);

            /* Submit DOF Request*/
            status = Fvid2_processRequest(dofObj->handle, inFrmList,
                outFrmList, FVID2_TIMEOUT_FOREVER);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxDmpacDofProcess: Failed to Submit Request\n");
                status = VX_FAILURE;
            }

            if (VX_SUCCESS == status)
            {
                /* Wait for Frame Completion */
                tivxEventWait(dofObj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

                status = Fvid2_getProcessedRequest(dofObj->handle,
                    inFrmList, outFrmList, 0);
                if (FVID2_SOK != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofProcess: Failed to Get Processed Request\n");
                    status = VX_FAILURE;
                }
            }
        }

        if (VX_SUCCESS == status)
        {
            /* Get Histogram */
            if(NULL != confidence_histogram_desc)
            {
                target_ptr = tivxMemShared2TargetPtr(
                            confidence_histogram_desc->mem_ptr.shared_ptr,
                            confidence_histogram_desc->mem_ptr.mem_heap_region);

                tivxMemBufferMap(target_ptr, confidence_histogram_desc->mem_size,
                                VX_MEMORY_TYPE_HOST, VX_READ_AND_WRITE);

                status = Fvid2_control(dofObj->handle,
                                       VHWA_M2M_IOCTL_DOF_GET_HISTOGRAM,
                                       (uint32_t *)target_ptr, NULL);
                if (FVID2_SOK != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofProcess: Histogram Request failed\n");
                    status = VX_FAILURE;
                }
                else
                {
                    status = VX_SUCCESS;
                }

                tivxMemBufferUnmap(target_ptr, confidence_histogram_desc->mem_size,
                                VX_MEMORY_TYPE_HOST, VX_READ_AND_WRITE);
            }

        }

        if(1 == dofObj->isFirstFrame)
        {
            /* This was the first iteration, so the Temopral predictor was
                turned off, Enable the Temporal predictor if configured */
            update_prms = 1;

            dofObj->isFirstFrame = 0u;
        }

        config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

        target_ptr = tivxMemShared2TargetPtr(
                            config_desc->mem_ptr.shared_ptr,
                            config_desc->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, config_desc->mem_size,
                            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;

        /* Check if configuration changes */
        if(memcmp(dofAppPrms, &dofObj->dofAppPrms,
            sizeof(tivx_dmpac_dof_params_t)) != 0)
        {
            update_prms = 1;
        }

        if(1 == update_prms)
        {
            status = tivxDmpacDofUpdateCfgPrms(dofObj, dofAppPrms);
        }

        tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
                                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    tivx_dmpac_dof_params_t          *dofAppPrms = NULL;
    Vhwa_M2mDofPrms                  *dofPrms = NULL;
    tivxDmpacDofObj                  *dofObj = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_pyramid_t          *input_current_desc;
    tivx_shared_mem_ptr_t            tBuffPtr;
    void                             *target_ptr;

    if ((num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofCreate: Required input parameter set to NULL\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        dofObj = tivxDmpacDofAllocObject(&gTivxDmpacDofInstObj);
        if (NULL != dofObj)
        {
            /* point to descriptors with correct type */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

            input_current_desc = (tivx_obj_desc_pyramid_t *)
                                obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];

            if(NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX])
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels + 1;
            }
            else
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels;
            }

            dofObj->isFirstFrame = 1;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                            "tivxDmpacDofCreate: May need to increase the value of VHWA_M2M_DOF_MAX_HANDLES in pdk/packages/ti/drv/vhwa/include/vhwa_m2mDof.h\n");
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    if (VX_SUCCESS == status)
    {
        Vhwa_m2mDofCreateArgsInit(&dofObj->createArgs);

        status = tivxEventCreate(&dofObj->waitForProcessCmpl);
        if (VX_SUCCESS == status)
        {
            dofObj->cbPrms.cbFxn   = tivxDmpacDofFrameComplCb;
            dofObj->cbPrms.appData = dofObj;

            dofObj->handle = Fvid2_create(FVID2_VHWA_M2M_DOF_DRV_ID,
                VHWA_M2M_DOF_DRV_INST_ID, (void *)&dofObj->createArgs,
                NULL, &dofObj->cbPrms);

            if (NULL == dofObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofCreate: Invalid Handle\n");
                status = VX_FAILURE;
            }
        }
    }

    /* Register Error Callback */
    if (VX_SUCCESS == status)
    {
        dofObj->errEvtPrms.errEvents =
            VHWA_DOF_RD_ERR | VHWA_DOF_WR_ERR |
            VHWA_DOF_MP0_RD_STATUS_ERR | VHWA_DOF_FOCO0_SL2_WR_ERR |
            VHWA_DOF_FOCO0_VBUSM_RD_ERR;
        dofObj->errEvtPrms.cbFxn     = tivxDmpacDofErrorCb;
        dofObj->errEvtPrms.appData   = dofObj;

        status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB, &dofObj->errEvtPrms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofCreate: Error CB registration failed\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }
    }

    /* Set DOF Configuration Parameters */
    if (VX_SUCCESS == status)
    {
        dofPrms = &dofObj->dofPrms;

        target_ptr = tivxMemShared2TargetPtr(
            config_desc->mem_ptr.shared_ptr,
            config_desc->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, config_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;

        tivxDmpacDofSetCfgPrms(dofPrms, dofAppPrms, obj_desc);

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           DOF paramerters needs to be reconfigured */
        memcpy(&dofObj->dofAppPrms, dofAppPrms, sizeof(tivx_dmpac_dof_params_t));

        status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dofObj->dofPrms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofCreate: Set parameters request failed\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }
        tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    /* Initialize and set the Confidence score parameters to default value */
    if (VX_SUCCESS == status)
    {
        Vhwa_m2mConfScoreParamInit(&dofObj->csPrms);

        status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS,
                                &dofObj->csPrms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofCreate: Set CS parameter reqeust failed\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }
    }

    /* Allocate intemediate buffers for Pyramid processing */
    if (VX_SUCCESS == status)
    {
        status = tivxMemBufferAlloc(&tBuffPtr, (dofObj->dofPrms.coreCfg.width *
                        dofObj->dofPrms.coreCfg.height), TIVX_MEM_EXTERNAL);
        if (VX_SUCCESS == status)
        {
            dofObj->inter_buff1 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                tBuffPtr.mem_heap_region);

            status = tivxMemBufferAlloc(&tBuffPtr, (dofObj->dofPrms.coreCfg.width *
                        dofObj->dofPrms.coreCfg.height), TIVX_MEM_EXTERNAL);
        }

        if (VX_SUCCESS == status)
        {
            dofObj->inter_buff2 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                tBuffPtr.mem_heap_region);
        }
    }


    if (VX_SUCCESS == status)
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
    vx_status       status = VX_SUCCESS;
    uint32_t        size;
    tivxDmpacDofObj *dofObj = NULL;

    if (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofDelete: Invalid Input\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&dofObj, &size);

        if ((VX_SUCCESS == status) && (NULL != dofObj) &&
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
    vx_status                         status = VX_SUCCESS;
    uint32_t                          size;
    tivxDmpacDofObj                   *dofObj = NULL;

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&dofObj, &size);

        if ((VX_SUCCESS == status) && (NULL != dofObj) &&
            (sizeof(tivxDmpacDofObj) == size))
        {
            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofControl: Invalid Input\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_NODE_DMPAC_DOF_CS_PARAMS:
            {
                status = tivxDmpacDofSetCsPrms(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_NODE_DMPAC_DOF_SET_HTS_BW_LIMIT_PARAMS:
            {
                status = tivxDmpacDofSetHtsBwLimit(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_NODE_DMPAC_DOF_GET_ERR_STATUS:
            {
                status = tivxDmpacDofGetErrStatusCmd(dofObj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofControl: Invalid Input\n");
                status = VX_FAILURE;
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
    tivx_obj_desc_image_t *img_desc)
{
    if (NULL != img_desc)
    {
        switch (img_desc->format)
        {
            case VX_DF_IMAGE_U8:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case VX_DF_IMAGE_U16:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            case TIVX_DF_IMAGE_P12:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
        }

        fmt->pitch[0]   = img_desc->imagepatch_addr[0].stride_y;
    }
}

static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    tivx_dmpac_dof_params_t *dofAppPrms, tivx_obj_desc_t *obj_desc[])
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

    tivxGetObjDescList(input_curr_desc->obj_desc_id,
                       (tivx_obj_desc_t**)img_current_desc,
                       input_curr_desc->num_levels);

    if(NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX])
    {
        /* Information is in base input */
        input_curr_base_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];

        dofPrms->tPrmdLvl = input_curr_desc->num_levels + 1;

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

    /* For first frame temporal input is not available */
    if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2)
    {
        dofPrms->bPredictor2 = DOF_PREDICTOR_NONE;
    }
    else if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1)
    {
        dofPrms->bPredictor1 = DOF_PREDICTOR_NONE;
    }

    dofPrms->coreCfg.horizontalSearchRange = dofAppPrms->horizontal_search_range;
    dofPrms->coreCfg.topSearchRange = dofAppPrms->vertical_search_range[0];
    dofPrms->coreCfg.bottomSearchRange = dofAppPrms->vertical_search_range[1];
    dofPrms->coreCfg.medianFilter = dofAppPrms->median_filter_enable;

    if(0 == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0;
        dofPrms->coreCfg.referenceCensusTransform = 0;
    }
    else if(1 == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0;
        dofPrms->coreCfg.referenceCensusTransform = 1;
    }
    else if(2 == dofAppPrms->motion_direction)
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

    dofPrms->coreCfg.lkConfidanceScore = dofAppPrms->enable_lk;

    if(NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX])
    {
        sof_map_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];

        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] =
                                sof_map_desc->imagepatch_addr[0].stride_y;

        dofPrms->coreCfg.enableSof   = 1u;

        dofPrms->coreCfg.maxMVsInSof = dofAppPrms->sof_max_pix_in_row;
        dofPrms->flowVectorHeight = dofAppPrms->sof_fv_height;
    }
    else
    {
        dofPrms->coreCfg.enableSof = 0u;
        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] = 0u;
    }

    if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1 ||
       DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2)
    {
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] =
                                    fv_out_desc->imagepatch_addr[0].stride_y;
    }
    else
    {
        dofPrms->coreCfg.enableSof = 0u;
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0u;
    }

    dofPrms->inOutImgFmt[0][DOF_OUTPUT].pitch[0U] =
                                    fv_out_desc->imagepatch_addr[0].stride_y;

    for(pyr_cnt = 0; pyr_cnt < dofPrms->tPrmdLvl; pyr_cnt++)
    {
        /* Set for Temporal and SOF image */
        if(0 != pyr_cnt)
        {
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0U;
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_SOF].pitch[0U] = 0U;
        }

        /* Set for reference and current image */
        fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_CURRENT_IMG];

        if(NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX])
        {
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        img_current_desc[pyr_cnt]->imagepatch_addr[0].stride_y;

            dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    img_current_desc[pyr_cnt]->imagepatch_addr[0].stride_y * 2u;
        }
        else
        {
            if(pyr_cnt > 0)
            {
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1]);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1]);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        img_current_desc[pyr_cnt-1]->imagepatch_addr[0].stride_y;

                dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    img_current_desc[pyr_cnt-1]->imagepatch_addr[0].stride_y * 2u;
            }
            else
            {
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        input_curr_base_desc->imagepatch_addr[0].stride_y;
            }
        }

        fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_CURRENT_IMG];

        dofPrms->focoPrms.shiftM1 = 0u;
        dofPrms->focoPrms.dir = 0u;
        dofPrms->focoPrms.round = 0u;

        if(fmt->ccsFormat == FVID2_CCSF_BITS8_PACKED)
        {
            dofPrms->focoPrms.shiftM1 = 4u;
        }
    }

    return;

}

static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                        tivx_dmpac_dof_params_t *dofAppPrms)
{
    vx_status                    status = VX_SUCCESS;

    if(NULL == dofAppPrms)
    {
        VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacDofUpdateCfgPrms: Invalid Input\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
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

        if(0 == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 0;
        }
        else if(1 == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 1;
        }
        else if(2 == dofAppPrms->motion_direction)
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
        dof_obj->dofPrms.coreCfg.lkConfidanceScore = dofAppPrms->enable_lk;

        status = Fvid2_control(dof_obj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dof_obj->dofPrms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofUpdateCfgPrms: Set Parameter request failed\n");
            status = VX_FAILURE;
        }
        else
        {
            memcpy(&dof_obj->dofAppPrms, dofAppPrms,
                                            sizeof(tivx_dmpac_dof_params_t));

            status = VX_SUCCESS;
        }
    }

    return (status);

}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    uint32_t                            idx;
    uint32_t                            cnt;
    vx_status                           status = VX_SUCCESS;
    tivx_dmpac_dof_cs_tree_params_t    *cs_prms;
    Dof_ConfScoreParam                  csPrms;
    void                               *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR,
                    "tivxDmpacDofSetCsPrms: Invalid Input\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(
            usr_data_obj->mem_ptr.shared_ptr,
            usr_data_obj->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

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

            status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS, &csPrms, NULL);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxDmpacDofSetCsPrms: Set CS parameter request failed\n");
                status = VX_FAILURE;
            }
            else
            {
                status = VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofSetCsPrms: Invalid Argument\n");
            status = VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofSetCsPrms: Null Argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofGetErrStatusCmd(tivxDmpacDofObj *dof_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        scalar_obj_desc->data.u32 = dof_obj->err_stat;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofGetErrStatus: Null argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                               tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                                status = VX_SUCCESS;
    Vhwa_HtsLimiter                          hts_limit;
    tivx_dmpac_dof_hts_bw_limit_params_t    *app_hts_prms;
    void                                    *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofSetHtsBwLimit: Invalid Argument\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(
            usr_data_obj->mem_ptr.shared_ptr,
            usr_data_obj->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_dmpac_dof_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_dmpac_dof_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_HTS_LIMIT, &hts_limit, NULL);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxDmpacDofSetHtsBwLimit: Set HTS limit request failed\n");
                status = VX_FAILURE;
            }
            else
            {
                status = VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxDmpacDofSetRdBwLimit: Invalid Argument\n");
            status = VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacDofSetRdBwLimit: Null Argument\n");
        status = VX_FAILURE;
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

