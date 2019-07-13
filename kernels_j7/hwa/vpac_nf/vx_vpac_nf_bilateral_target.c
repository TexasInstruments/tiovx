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
#include "tivx_kernel_vpac_nf_bilateral.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include <math.h>

#include "ti/drv/vhwa/include/vhwa_m2mNf.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define LUT_ROWS 5

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    tivx_vpac_nf_bilateral_params_t     nfBilateralParams;
    Vhwa_M2mNfCreatePrms                createPrms;
    Vhwa_M2mNfConfig                    nf_cfg;
    Nf_WgtTableConfig                   wgtTbl;
    Fvid2_Handle                        handle;
    tivx_event                          waitForProcessCmpl;
    Nf_ErrEventParams                   errEvtPrms;
    
    Fvid2_Frame                         inFrm;
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;
    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;

    uint32_t                            err_stat;
} tivxVpacNfBilateralObj;

typedef struct
{
    tivx_mutex lock;
    tivxVpacNfBilateralObj nfBilateralObj[VHWA_M2M_NF_MAX_HANDLES];
} tivxVpacNfBilateralInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxVpacNfBilateralObj *tivxVpacNfBilateralAllocObject(
       tivxVpacNfBilateralInstObj *instObj);
static void tivxVpacNfBilateralFreeObject(
       tivxVpacNfBilateralInstObj *instObj, tivxVpacNfBilateralObj *nf_bilateral_obj);
static void tivxVpacNfSetFmt(Fvid2_Format *fmt,
    tivx_obj_desc_image_t *img_desc);
static void tivxVpacNfBilateralGenerateLut(uint8_t subRangeBits, double *sigma_s,
    double *sigma_r, uint32_t *i_lut);
static uint32_t tivxVpacNfBilateralGenerateLutCoeffs(uint8_t mode,uint8_t inp_bitw,
    uint8_t filtSize, double sigma_s, double sigma_r, double *f_wt_lut, uint8_t out_bitw,
    uint32_t *i_wt_lut_spatial, uint32_t *i_wt_lut_full);
static void tivxVpacNfBilateralInterleaveTables(uint32_t **i_lut, uint8_t numTables,
    uint32_t rangeLutEntries);
static uint32_t getSubRangeBits(uint16_t i);
static vx_status tivxVpacNfBilateralSetHtsLimitCmd(
    tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralSetCoeff(tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralGetErrStatusCmd(tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
           
int32_t tivxVpacNfBilateralFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxVpacNfBilateralErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_vpac_nf_bilateral_target_kernel = NULL;

tivxVpacNfBilateralInstObj gTivxVpacNfBilateralInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacNfBilateral(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_NF, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
        
        vx_vpac_nf_bilateral_target_kernel = tivxAddTargetKernelByName(
                    TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                    target_name,
                    tivxVpacNfBilateralProcess,
                    tivxVpacNfBilateralCreate,
                    tivxVpacNfBilateralDelete,
                    tivxVpacNfBilateralControl,
                    NULL);
        if (NULL != vx_vpac_nf_bilateral_target_kernel)
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVpacNfBilateralInstObj.lock);
            if (VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxAddTargetKernelVpacNfBilateral: Failed to create Mutex\n");
            }
            else
            {
                memset(&gTivxVpacNfBilateralInstObj.nfBilateralObj, 0x0U,
                    sizeof(tivxVpacNfBilateralObj) * VHWA_M2M_NF_MAX_HANDLES);
            }
        }
        else
        {
            status = VX_FAILURE;

            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR,
                "tivxAddTargetKernelVpacNfBilateral: Failed to Add NF Bilateral TargetKernel\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxAddTargetKernelVpacNfBilateral: Invalid CPU\n");
    }
}

void tivxRemoveTargetKernelVpacNfBilateral(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_nf_bilateral_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_vpac_nf_bilateral_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxRemoveTargetKernelVpacNfBilateral: Failed to Remove Nf TargetKernel\n");
    }
    if (NULL != gTivxVpacNfBilateralInstObj.lock)
    {
        tivxMutexDelete(&gTivxVpacNfBilateralInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    uint32_t                          size;
    void                             *src_target_ptr;
    void                             *sigmas_target_ptr;
    void                             *configuration_target_ptr;
    void                             *dst_target_ptr;
    tivxVpacNfBilateralObj           *nf_bilateral_obj = NULL;
    tivx_obj_desc_image_t            *src;
    tivx_obj_desc_user_data_object_t *sigmas_desc;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t            *dst;
    Fvid2_FrameList                  *inFrmList;
    Fvid2_FrameList                  *outFrmList;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVpacNfBilateralProcess: Invalid Descriptor\n");
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&nf_bilateral_obj, &size);

        if (VX_SUCCESS != status)
        {
			VX_PRINT(VX_ZONE_ERROR, "tivxVpacNfBilateralProcess: Null Desc\n");
        }
        else if (sizeof(tivxVpacNfBilateralObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacNfBilateralProcess: Incorrect object size\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        inFrmList = &nf_bilateral_obj->inFrmList;
        outFrmList = &nf_bilateral_obj->outFrmList;
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
        sigmas_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        configuration_target_ptr = tivxMemShared2TargetPtr(
            configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_heap_region);
        sigmas_target_ptr = tivxMemShared2TargetPtr(
            sigmas_desc->mem_ptr.shared_ptr, sigmas_desc->mem_ptr.mem_heap_region);
        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(configuration_target_ptr,
            configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(sigmas_target_ptr,
            sigmas_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        /* Initialize NF Input Frame List */
        inFrmList->frames[0U] =
		    &nf_bilateral_obj->inFrm;
        inFrmList->numFrames = 1U;
		
		nf_bilateral_obj->inFrm.addr[0U] = (uint64_t) src_target_ptr;

        /* Initialize NF Output Frame List */
        outFrmList->frames[0U] = &nf_bilateral_obj->outFrm;
        outFrmList->numFrames = 1U;
        
		nf_bilateral_obj->outFrm.addr[0U] = (uint64_t) dst_target_ptr;

        /* Submit NF Request*/
        status = Fvid2_processRequest(nf_bilateral_obj->handle, inFrmList,
            outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralProcess: Failed to Submit Request\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Wait for Frame Completion */
        tivxEventWait(nf_bilateral_obj->waitForProcessCmpl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        status = Fvid2_getProcessedRequest(nf_bilateral_obj->handle,
            inFrmList, outFrmList, 0);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralProcess: Failed to Get Processed Request\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(configuration_target_ptr,
            configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(sigmas_target_ptr,
            sigmas_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;

    tivxVpacNfBilateralObj           *nf_bilateral_obj = NULL;
    Vhwa_M2mNfConfig                 *nf_cfg = NULL;
    tivx_vpac_nf_bilateral_params_t  *params = NULL;
    tivx_vpac_nf_bilateral_sigmas_t  *sigmas = NULL;
    tivx_obj_desc_user_data_object_t *params_array = NULL;
    tivx_obj_desc_user_data_object_t *sigmas_array = NULL;
    tivx_obj_desc_image_t            *src;
    tivx_obj_desc_image_t            *dst;
    void                             *params_array_target_ptr = NULL;
    void                             *sigmas_array_target_ptr = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if (VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralCreate: Required input parameter set to NULL\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        nf_bilateral_obj = tivxVpacNfBilateralAllocObject(&gTivxVpacNfBilateralInstObj);
        if (NULL != nf_bilateral_obj)
        {
            params_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
            sigmas_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];
            src = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
            dst = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Failed to Alloc Nf Bilateral Object\n");
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    if (VX_SUCCESS == status)
    {
        Vhwa_M2mNfCreatePrmsInit(&nf_bilateral_obj->createPrms);

        status = tivxEventCreate(&nf_bilateral_obj->waitForProcessCmpl);
        if (VX_SUCCESS == status)
        {
            nf_bilateral_obj->cbPrms.cbFxn   = tivxVpacNfBilateralFrameComplCb;
            nf_bilateral_obj->cbPrms.appData = nf_bilateral_obj;

            nf_bilateral_obj->handle = Fvid2_create(FVID2_VHWA_M2M_NF_DRV_ID,
                VHWA_M2M_NF_DRV_INST_ID, (void *)&nf_bilateral_obj->createPrms,
                NULL, &nf_bilateral_obj->cbPrms);

            if (NULL == nf_bilateral_obj->handle)
            {
				VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Failed to Alloc Nf Bilateral Object\n");
                status = VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Failed to allocate Event\n");
        }
    }

    /* Register Error Callback */
    if (VX_SUCCESS == status)
    {
        nf_bilateral_obj->errEvtPrms.errEvents = VHWA_NF_RD_ERR | VHWA_NF_WR_ERR;
        nf_bilateral_obj->errEvtPrms.cbFxn     = tivxVpacNfBilateralErrorCb;
        nf_bilateral_obj->errEvtPrms.appData   = nf_bilateral_obj;

        status = Fvid2_control(nf_bilateral_obj->handle,
            IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB, &nf_bilateral_obj->errEvtPrms, NULL);
        if (FVID2_SOK != status)
        {
			VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Failed to Register Error Callback\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }
    }

    if (VX_SUCCESS == status)
    {
        nf_cfg = &nf_bilateral_obj->nf_cfg;

        params_array_target_ptr = tivxMemShared2TargetPtr(
            params_array->mem_ptr.shared_ptr, params_array->mem_ptr.mem_heap_region);
        sigmas_array_target_ptr = tivxMemShared2TargetPtr(
            sigmas_array->mem_ptr.shared_ptr, sigmas_array->mem_ptr.mem_heap_region);

        tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(sigmas_array_target_ptr, sigmas_array->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        params = (tivx_vpac_nf_bilateral_params_t *)params_array_target_ptr;
        sigmas = (tivx_vpac_nf_bilateral_sigmas_t *)sigmas_array_target_ptr;

        /* Initialize NF Config with defaults */
        Nf_ConfigInit(&nf_cfg->nfCfg);

        /* Set NF Config parameters - centralPixelWeight set in tivxVpacNfBilateralProcess */
        nf_cfg->nfCfg.filterMode = NF_FILTER_MODE_BILATERAL;
        nf_cfg->nfCfg.tableMode = params->adaptive_mode;
        nf_cfg->nfCfg.skipMode = params->params.output_pixel_skip;
        nf_cfg->nfCfg.interleaveMode = params->params.input_interleaved;
        nf_cfg->nfCfg.outputShift = params->params.output_downshift;
        nf_cfg->nfCfg.outputOffset = params->params.output_offset;
        nf_cfg->nfCfg.numSubTables = getSubRangeBits(sigmas->num_sigmas);
        nf_cfg->nfCfg.subTableIdx = params->sub_table_select;
        nf_cfg->nfCfg.centralPixelWeight = 255u;
        nf_bilateral_obj->wgtTbl.filterMode = NF_FILTER_MODE_BILATERAL;
                
        tivxVpacNfBilateralGenerateLut(getSubRangeBits(sigmas->num_sigmas), sigmas->sigma_space, sigmas->sigma_range,
            nf_bilateral_obj->wgtTbl.blFilterLut);
    
        tivxVpacNfSetFmt(&nf_cfg->inFmt, src);
        tivxVpacNfSetFmt(&nf_cfg->outFmt, dst);

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           VPAC NF parameters needs to be reconfigured */

        memcpy(&nf_bilateral_obj->nfBilateralParams, params, sizeof(tivx_vpac_nf_bilateral_params_t));

        status = Fvid2_control(nf_bilateral_obj->handle,
            IOCTL_VHWA_M2M_NF_SET_PARAMS, &nf_bilateral_obj->nf_cfg, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Set parameters request failed\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }

        /* Set NF coeff */
        status = Fvid2_control(nf_bilateral_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
            &nf_bilateral_obj->wgtTbl, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralCreate: Set coeffs request failed\n");
            status = VX_FAILURE;
        }
        else
        {
            status = VX_SUCCESS;
        }

        tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
        
    if (VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, nf_bilateral_obj,
            sizeof(tivxVpacNfBilateralObj));
    }
    else
    {
        if (NULL != nf_bilateral_obj)
        {
            if (NULL != nf_bilateral_obj->handle)
            {
                Fvid2_delete(nf_bilateral_obj->handle, NULL);
                nf_bilateral_obj->handle = NULL;
            }

            if (NULL != nf_bilateral_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&nf_bilateral_obj->waitForProcessCmpl);
            }

            tivxVpacNfBilateralFreeObject(&gTivxVpacNfBilateralInstObj, nf_bilateral_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = VX_SUCCESS;
    uint32_t                 size;
    tivxVpacNfBilateralObj    *nf_bilateral_obj = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&nf_bilateral_obj, &size);

        if ((VX_SUCCESS == status) && (NULL != nf_bilateral_obj) &&
            (sizeof(tivxVpacNfBilateralObj) == size))
        {
            if (NULL != nf_bilateral_obj->handle)
            {
                Fvid2_delete(nf_bilateral_obj->handle, NULL);
                nf_bilateral_obj->handle = NULL;
            }

            if (NULL != nf_bilateral_obj->waitForProcessCmpl)
            {
                tivxEventDelete(&nf_bilateral_obj->waitForProcessCmpl);
            }

            tivxVpacNfBilateralFreeObject(&gTivxVpacNfBilateralInstObj, nf_bilateral_obj);
        }
        else
        {
            status = VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralDelete: Invalid Target Instance Context\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralDelete: Invalid Descriptor\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = VX_SUCCESS;
    uint32_t                          size;
    tivxVpacNfBilateralObj             *nf_bilateral_obj = NULL;

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&nf_bilateral_obj, &size);

        if (VX_SUCCESS != status)
        {
		    VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralControl: Failed to get Target Kernel Instance Context\n");
        }
        else if ((NULL == nf_bilateral_obj) ||
            (sizeof(tivxVpacNfBilateralObj) != size))
        {
		    VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralControl: Wrong Size for Nf Bilateral Obj\n");
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_NF_SET_HTS_LIMIT:
            {
                status = tivxVpacNfBilateralSetHtsLimitCmd(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_SET_COEFF:
            {
                status = tivxVpacNfBilateralSetCoeff(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_GET_ERR_STATUS:
            {
                status = tivxVpacNfBilateralGetErrStatusCmd(nf_bilateral_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacNfBilateralControl: Invalid Node Command Id\n");
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

static tivxVpacNfBilateralObj *tivxVpacNfBilateralAllocObject(
       tivxVpacNfBilateralInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacNfBilateralObj *nf_bilateral_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);
    
    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->nfBilateralObj[cnt].isAlloc)
        {
            nf_bilateral_obj = &instObj->nfBilateralObj[cnt];
            memset(nf_bilateral_obj, 0x0, sizeof(tivxVpacNfBilateralObj));
            instObj->nfBilateralObj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (nf_bilateral_obj);
}

static void tivxVpacNfBilateralFreeObject(tivxVpacNfBilateralInstObj *instObj,
    tivxVpacNfBilateralObj *nf_bilateral_obj)
{
    uint32_t cnt;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
    {
        if (nf_bilateral_obj == &instObj->nfBilateralObj[cnt])
        {
            nf_bilateral_obj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);
}

static void tivxVpacNfSetFmt(Fvid2_Format *fmt,
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
            case VX_DF_IMAGE_S16:
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
            default:
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacNfSetFmt: Invalid Vx Image Format\n");
                break;
            }
        }

        fmt->width     = img_desc->imagepatch_addr[0U].dim_x;
        fmt->height    = img_desc->imagepatch_addr[0U].dim_y;
        fmt->pitch[0U] = img_desc->imagepatch_addr[0U].stride_y;
    }
}

static void tivxVpacNfBilateralGenerateLut(uint8_t subRangeBits, double *sigma_s,
    double *sigma_r, uint32_t *i_lut)
{
    uint32_t numTables = 1 << subRangeBits;
    uint8_t tableNum;
    uint32_t rangeLutEntries = 256 >> subRangeBits;
    double   f_lut[LUT_ROWS * 256];

    for (tableNum = 0; tableNum < numTables; tableNum++)
    {
        double s_sigma = sigma_s[tableNum];
        double r_sigma = sigma_r[tableNum];

        /*-----------------------------------------------------------------*/
        /* Generate fixed point LUT values, with index to LUT being        */
        /* pixel differences.                                              */
        /*-----------------------------------------------------------------*/
        tivxVpacNfBilateralGenerateLutCoeffs
            (
            0,
            8 - subRangeBits,
            5,
            s_sigma,
            r_sigma,
            f_lut,
            8,
            NULL,
            &i_lut[tableNum * LUT_ROWS * rangeLutEntries]
            );
    }

    if (numTables > 1)
    {
        tivxVpacNfBilateralInterleaveTables(&i_lut, numTables, rangeLutEntries);
    }
}

static uint32_t tivxVpacNfBilateralGenerateLutCoeffs(uint8_t mode,uint8_t inp_bitw,
    uint8_t filtSize, double sigma_s, double sigma_r, double *f_wt_lut, uint8_t out_bitw,
    uint32_t *i_wt_lut_spatial, uint32_t *i_wt_lut_full)
{
    int row, col;
    int lut_w, lut_h;
    int lutSize = 1 << inp_bitw;
    int referenceSize = 12;
    uint8_t numspatialDistances;

    double wt_s;
    double wt_r;
    double wt_sum;
    double max = 0;

    /* Translate filter size (5x5, 3x3, 1x1) into spatialDistances */
    if (filtSize == 3)
        numspatialDistances = 2;
    else if (filtSize == 1)
        numspatialDistances = 0;
    else
        numspatialDistances = 5;

    /*---------------------------------------------------------------------*/
    /* Compute LUT width and height.                                       */
    /*---------------------------------------------------------------------*/
    memset(f_wt_lut, 0, LUT_ROWS * lutSize*sizeof(double));

    lut_w = (1 << inp_bitw);
    lut_h = (numspatialDistances);

    /*---------------------------------------------------------------------*/
    /* Actual doubleing pt. LUT creation for Bilateral filter.              */
    /*---------------------------------------------------------------------*/

    /* If the space sigma is 0, then table should remain all zeros */
    if (sigma_s)
    {
        /* Index of the space distance from the center pixel, 0-4 */
        const uint8_t spaceWeightIndex[25] = {  4, 3, 2, 3, 4,
                                                3, 1, 0, 1, 3,
                                                2, 0, 0, 0, 2,
                                                3, 1, 0, 1, 3,
                                                4, 3, 2, 3, 4  };

        /* This distance-squared mapping of each index (above) in a 5x5 to the center pixel */
        const double distanceValuesSquared[] =
        {
            1.0, // 0: 1^2 + 0^2
            2.0, // 1: 1^2 + 1^2
            4.0, // 2: 2^2 + 0^2
            5.0, // 3: 2^2 + 1^2
            8.0  // 4: 2^2 + 2^2
        };

        /* If the range sigma is 0, bilateral doesn't make sense, so instead generic mode weights should be generated */
        if (sigma_r == 0)
        {
            /* Generate generic mode weights using gaussian curve from sigma_s */
            for (row = 0; row < 25; row++)
            {
                if (spaceWeightIndex[row] < numspatialDistances)
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2 * sigma_s * sigma_s);
                    f_wt_lut[row] = exp(-wt_s);
                }
            }
            for (row = 13; row < 25; row++)
            {
                if (spaceWeightIndex[row] < numspatialDistances)
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2 * sigma_s * sigma_s);
                    f_wt_lut[row-1] = exp(-wt_s);
                }
            }
            /* Overwrite center pixel weight to be 1 */
            f_wt_lut[12] = max = 1.0;
        }
        else {
            if (mode == 0) /* Bilateral Filter Weights */
            {
                for (col = 0; col < lut_w; col++)
                {
                    // Only generate the lut values across the range that are needed
                    int col_mod = col*(1 << (referenceSize - inp_bitw));
                    // gaussian_r = exp(-(x^2 / (2*sigma_r^2))
                    wt_r = (col_mod * col_mod) / (2 * sigma_r * sigma_r);

                    for (row = 0; row < lut_h; row++)
                    {
                        // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                        wt_s = distanceValuesSquared[row] / (2 * sigma_s * sigma_s);
                        wt_sum = wt_s + wt_r;
                        f_wt_lut[(row * lut_w) + col] = exp(-wt_sum);
                        //printf("%f\n", f_wt_lut[(row * lut_w) + col]);
                    }
                }
                max = 1.0;
            }
            else if (mode == 1 || mode == 2) /* Passthrough Weights */
            {
                f_wt_lut[0] = 1.0;
                max = 1.0;
            }
            else if (mode == 3) /* Highest Value Weights */
            {
                for (col = 0; col < lut_w; col++)
                    for (row = 0; row < lut_h; row++)
                        f_wt_lut[(row * lut_w) + col] = 1.0;
                max = 1.0;
            }
            else
                return 1;
        }
    }

    /*---------------------------------------------------------------------*/
    /* Fixed point LUT creation here.                                      */
    /*---------------------------------------------------------------------*/
    if (i_wt_lut_full){
        int i;
        memset(i_wt_lut_full, 0, LUT_ROWS * lutSize*sizeof(uint32_t));
        if (mode == 2) {
        }
        else {
            for (i = 0; i < LUT_ROWS * lutSize; i++) {
                i_wt_lut_full[i] = (uint32_t)((double)(f_wt_lut[i] / max) * (double)((1 << out_bitw) - 1) + 0.5);
            }
        }
    }

    /* In case spatial lut needs to be generated separatly */
    if (i_wt_lut_spatial){
        memset(i_wt_lut_spatial, 0, LUT_ROWS * sizeof(i_wt_lut_spatial[0]));
        for (row = 0; row < lut_h; row++)
        {
            i_wt_lut_spatial[row] = i_wt_lut_full[row * lut_w];
        }
    }
    return 0;
}

static void tivxVpacNfBilateralInterleaveTables(uint32_t **i_lut, uint8_t numTables,
    uint32_t rangeLutEntries)
{
    uint32_t *oldLut = *i_lut;
    uint32_t newLut[LUT_ROWS*256];
    uint32_t i, j;

    for (j = 0; j < numTables; j++)
        for (i = 0; i < LUT_ROWS * rangeLutEntries; i++)
        {
            newLut[numTables*i + j] = oldLut[j * LUT_ROWS * rangeLutEntries + i];
        }

    memcpy(oldLut, newLut, LUT_ROWS*256*sizeof(uint32_t));
}

static uint32_t getSubRangeBits(uint16_t i)
{
    uint32_t out = 0u;
    while (i >>= 1)
    {
        out++;
    }
    return out;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacNfBilateralSetHtsLimitCmd(
    tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                                status = VX_SUCCESS;
    Vhwa_HtsLimiter                          hts_limit;
    tivx_vpac_nf_hts_bw_limit_params_t      *app_hts_prms;
    void                                    *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacNfBilateralSetHtsLimitCmd: Invalid Argument\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(
            usr_data_obj->mem_ptr.shared_ptr,
            usr_data_obj->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(tivx_vpac_nf_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_vpac_nf_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            status = Fvid2_control(nf_bilateral_obj->handle,
                        IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT, &hts_limit, NULL);
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacNfBilateralSetHtsLimitCmd: Set HTS limit request failed\n");
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
                "tivxVpacNfBilateralSetHtsLimitCmd: Invalid Argument\n");
            status = VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralSetHtsLimitCmd: Null Argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxVpacNfBilateralSetCoeff(tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = VX_SUCCESS;
    Nf_WgtTableConfig                *wgtTbl = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(
            usr_data_obj->mem_ptr.shared_ptr,
            usr_data_obj->mem_ptr.mem_heap_region);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (sizeof(Nf_WgtTableConfig) ==
                usr_data_obj->mem_size)
        {
            wgtTbl = (Nf_WgtTableConfig *)target_ptr;
            status = Fvid2_control(nf_bilateral_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
                           wgtTbl, NULL);
                           
            if (FVID2_SOK != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacNfBilateralSetCoeff: Set coeff request failed\n");
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
                "tivxVpacNfBilateralSetCoeff: Invalid Argument\n");
            status = VX_FAILURE;
        }
        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralSetCoeff: Null Argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxVpacNfBilateralGetErrStatusCmd(tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = VX_SUCCESS;

    if (NULL != scalar_obj_desc)
    {
        scalar_obj_desc->data.u32 = nf_bilateral_obj->err_stat;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralGetErrStatusCmd: Null argument\n");
        status = VX_FAILURE;
    }

    return (status);
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacNfBilateralFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacNfBilateralObj *nf_bilateral_obj = (tivxVpacNfBilateralObj *)appData;

    if (NULL != nf_bilateral_obj)
    {
        tivxEventPost(nf_bilateral_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

void tivxVpacNfBilateralErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacNfBilateralObj *nf_bilateral_obj = (tivxVpacNfBilateralObj *)appData;

    if (NULL != nf_bilateral_obj)
    {
        if(errEvents & VHWA_NF_RD_ERR)
        {
            /* SL2 RD Error */
            errEvents = (errEvents & (~VHWA_NF_RD_ERR));
        }
        else if(errEvents & VHWA_NF_WR_ERR)
        {
            /* SL2 WR Error */
            errEvents = (errEvents & (~VHWA_NF_WR_ERR));
        }
    }
}
