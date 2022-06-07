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
#include <tivx_kernel_halfscale_gaussian.h>
#include "tivx_hwa_kernels.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_msc_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"

#include "ti/drv/vhwa/include/vhwa_m2mMsc.h"

#include "utils/perf_stats/include/app_perf_stats.h"

#include <vx_vpac_msc_scale_coeff.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* There are two target kernels supported by this implemenation,
 * MSC
 * two is OpenVX Gaussian Pyramid node
 */
#define TIVX_VPAC_MSC_SCALE_NUM_INST                (VHWA_M2M_MSC_MAX_INST * 2u)

#define TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_TARGET    (0x1000U)

#define TIVX_VPAC_MSC_SCALE_IMAGE_TARGET            (0x2000U)

#define TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_START_IDX (0u)
#define TIVX_VPAC_MSC_SCALE_IMAGE_START_IDX         (2u)

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
    Fvid2_Frame             outFrm;
    Fvid2_CbParams          cbPrms;

    tivxVpacMscScaleInstObj *inst_obj;

    uint32_t                sc_map_idx;
    Msc_Coeff               coeffCfg;

} tivxVpacMscHsgObj;


struct tivxVpacMscScaleInstObj_t
{
    tivx_mutex              lock;
    tivxVpacMscHsgObj       msc_obj[VHWA_M2M_MSC_MAX_HANDLES];

    tivx_target_kernel      target_kernel;

    uint32_t                alloc_sc_fwd_dir;

    uint32_t                msc_drv_inst_id;

    uint32_t                target_type;

    /*! HWA Performance ID */
    app_perf_hwa_id_t       hwa_perf_id;
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

/* Local Functions */
static tivxVpacMscHsgObj *tivxVpacMscScaleAllocObject(
    tivxVpacMscScaleInstObj *instObj);
static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscHsgObj *msc_obj);
static void tivxVpacMscScaleSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc, const tivx_obj_desc_image_t *out_img_desc,
    uint32_t target_type);
static void tivxVpacMscScaleSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);

/* Driver Callback */
int32_t tivxVpacMscScaleFrameComplCb(Fvid2_Handle handle, void *appData);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

tivxVpacMscScaleInstObj gTivxVpacMscScaleInstObj[TIVX_VPAC_MSC_SCALE_NUM_INST];

/*! Keeps track of the Single phase coefficients */
int32_t gVpacMscHsgSpCoeff[3U][MSC_MAX_TAP] = TIVX_VPAC_MSC_SP_SCALE_COEFFICIENTS;
/*! Keeps track of the Multi phase coefficients for Nearest Neighbor
 *  & Bilinear Interpolation */
int32_t gVpacMscScaleMpCoeff[2U][64u*5u] = TIVX_VPAC_MSC_MP_SCALE_COEFFICIENTS;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscHalfScaleGaussian(void)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 cnt;
    uint32_t                 inst_start;
    char                     target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                  self_cpu;
    tivxVpacMscScaleInstObj *inst_obj;

    inst_start = TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_START_IDX;
    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if ((self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0))
#else
    if ( (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
#if defined(SOC_J784S4)
        || ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
#endif
        )
#endif
    {
        /* Reset all values to 0 */
        memset(&gTivxVpacMscScaleInstObj[inst_start], 0x0,
            sizeof(tivxVpacMscScaleInstObj) * VHWA_M2M_MSC_MAX_INST);

        for (cnt = 0u; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            inst_obj = &gTivxVpacMscScaleInstObj[inst_start + cnt];

            if (0u == cnt)
            {
                #ifdef SOC_AM62A
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                #else
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                #endif
                {
                    strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
                        TIVX_TARGET_MAX_NAME);
                }
                #if defined(SOC_J784S4)
                else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                {
                    strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
                        TIVX_TARGET_MAX_NAME);
                }
                #endif
            }
            else
            {
                #ifdef SOC_AM62A
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                #else
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                #endif
                {
                    strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
                        TIVX_TARGET_MAX_NAME);
                }
                #if defined(SOC_J784S4)
                else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                {
                    strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
                        TIVX_TARGET_MAX_NAME);
                }
                #endif
            }

            inst_obj->target_kernel = tivxAddTargetKernel(
                                (vx_enum)VX_KERNEL_HALFSCALE_GAUSSIAN,
                                target_name,
                                tivxVpacMscScaleProcess,
                                tivxVpacMscScaleCreate,
                                tivxVpacMscScaleDelete,
                                NULL,
                                NULL);
            if (NULL != inst_obj->target_kernel)
            {
                /* Allocate lock semaphore */
                status = tivxMutexCreate(&inst_obj->lock);
                if ((vx_status)VX_SUCCESS != status)
                {
                    tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Semaphore\n");
                }
                else
                {
                    /* Initialize Instance Object */
                    if (0u == cnt)
                    {
                        #ifdef SOC_AM62A
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                        #else
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                        #endif
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC0;
                        }
                        #if defined(SOC_J784S4)
                        else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC0;
                        }
                        #endif
                        inst_obj->alloc_sc_fwd_dir = 1U;
                    }
                    else
                    {
                        #ifdef SOC_AM62A
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                        #else
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                        #endif
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC1;
                        }
                        #if defined(SOC_J784S4)
                        else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC1;
                        }
                        #endif
                        inst_obj->alloc_sc_fwd_dir = 0U;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;

                /* TODO: how to handle this condition */
                VX_PRINT(VX_ZONE_ERROR, "Failed to Add MSC TargetKernel\n");
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                inst_obj->target_type = TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_TARGET;
            }
        }

        /* Clean up allocated resources */
        if ((vx_status)VX_SUCCESS != status)
        {
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                inst_obj = &gTivxVpacMscScaleInstObj[cnt];
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

void tivxRemoveTargetKernelVpacMscHalfScaleGaussian(void)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 cnt;
    uint32_t                 inst_start;
    tivxVpacMscScaleInstObj *inst_obj;

    inst_start = TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscScaleInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Msc TargetKernel\n");
        }

        if (NULL != inst_obj->lock)
        {
            tivxMutexDelete(&inst_obj->lock);
        }
    }
}

void tivxAddTargetKernelVpacMscScale(void)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 cnt;
    char                     target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                  self_cpu;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                 inst_start;

    inst_start = TIVX_VPAC_MSC_SCALE_IMAGE_START_IDX;
    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if ((self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0))
#else
    if ( (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
#if defined(SOC_J784S4)
        || ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
#endif
        )
#endif
    {
        /* Reset all values to 0 */
        memset(&gTivxVpacMscScaleInstObj[inst_start], 0x0,
            sizeof(tivxVpacMscScaleInstObj) * VHWA_M2M_MSC_MAX_INST);

        for (cnt = 0u; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            inst_obj = &gTivxVpacMscScaleInstObj[inst_start + cnt];

            if (0u == cnt)
            {
                #ifdef SOC_AM62A
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                #else
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                #endif
                {
                    strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
                        TIVX_TARGET_MAX_NAME);
                }
                #if defined(SOC_J784S4)
                else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                {
                    strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
                        TIVX_TARGET_MAX_NAME);
                }
                #endif
            }
            else
            {
                #ifdef SOC_AM62A
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                #else
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                #endif
                {
                    strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
                        TIVX_TARGET_MAX_NAME);
                }
                #if defined(SOC_J784S4)
                else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                {
                    strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
                        TIVX_TARGET_MAX_NAME);
                }
                #endif
            }

            inst_obj->target_kernel = tivxAddTargetKernel(
                                (vx_enum)VX_KERNEL_SCALE_IMAGE,
                                target_name,
                                tivxVpacMscScaleProcess,
                                tivxVpacMscScaleCreate,
                                tivxVpacMscScaleDelete,
                                NULL,
                                NULL);
            if (NULL != inst_obj->target_kernel)
            {
                /* Allocate lock semaphore */
                status = tivxMutexCreate(&inst_obj->lock);
                if ((vx_status)VX_SUCCESS != status)
                {
                    tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Semaphore\n");
                }
                else
                {
                    /* Initialize Instance Object */
                    if (0u == cnt)
                    {
                        #ifdef SOC_AM62A
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                        #else
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                        #endif
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC0;
                        }
                        #if defined(SOC_J784S4)
                        else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC0;
                        }
                        #endif
                        inst_obj->alloc_sc_fwd_dir = 1U;
                    }
                    else
                    {
                        #ifdef SOC_AM62A
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                        #else
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                        #endif
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC1;
                        }
                        #if defined(SOC_J784S4)
                        else if ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC1;
                        }
                        #endif
                        inst_obj->alloc_sc_fwd_dir = 0U;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;

                /* TODO: how to handle this condition */
                VX_PRINT(VX_ZONE_ERROR, "Failed to Add MSC TargetKernel\n");
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                inst_obj->target_type = TIVX_VPAC_MSC_SCALE_IMAGE_TARGET;
            }
        }

        /* Clean up allocated resources */
        if ((vx_status)VX_SUCCESS != status)
        {
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                inst_obj = &gTivxVpacMscScaleInstObj[cnt];
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

void tivxRemoveTargetKernelVpacMscScale(void)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 cnt;
    uint32_t                 inst_start;
    tivxVpacMscScaleInstObj *inst_obj;

    inst_start = TIVX_VPAC_MSC_SCALE_IMAGE_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscScaleInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Msc TargetKernel\n");
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
    tivxVpacMscHsgObj       *msc_obj = NULL;
    tivx_obj_desc_image_t   *in_img_desc = NULL;
    tivx_obj_desc_image_t   *out_img_desc = NULL;
    tivxVpacMscScaleInstObj *inst_obj = NULL;
    tivx_target_kernel       target_kernel = NULL;
    Fvid2_Format            *fmt = NULL;
    Msc_ScConfig            *sc_cfg = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        target_kernel = tivxTargetKernelInstanceGetKernel(kernel);
        if (NULL == target_kernel)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get Target Kernel\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
        }
        else
        {
            inst_obj = NULL;
            for (cnt = 0u; cnt < TIVX_VPAC_MSC_SCALE_NUM_INST; cnt ++)
            {
                if (target_kernel == gTivxVpacMscScaleInstObj[cnt].target_kernel)
                {
                    inst_obj = &gTivxVpacMscScaleInstObj[cnt];
                    break;
                }
            }

            if (NULL == inst_obj)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Target Kernel\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        msc_obj = tivxVpacMscScaleAllocObject(inst_obj);
        if (NULL != msc_obj)
        {
            in_img_desc = (tivx_obj_desc_image_t *)obj_desc
                [TIVX_KERNEL_HALFSCALE_GAUSSIAN_INPUT_IDX];
            out_img_desc = (tivx_obj_desc_image_t *)obj_desc
                [TIVX_KERNEL_HALFSCALE_GAUSSIAN_OUTPUT_IDX];

            /* Initialize Msc object */
            msc_obj->inst_obj = inst_obj;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Handle Object, increase VHWA_M2M_MSC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            Vhwa_M2mMscCreatePrmsInit(&msc_obj->createArgs);

            status = tivxEventCreate(&msc_obj->wait_for_compl);
            if ((vx_status)VX_SUCCESS == status)
            {
                msc_obj->cbPrms.cbFxn   = tivxVpacMscScaleFrameComplCb;
                msc_obj->cbPrms.appData = msc_obj;

                msc_obj->handle = Fvid2_create(FVID2_VHWA_M2M_MSC_DRV_ID,
                    inst_obj->msc_drv_inst_id, &msc_obj->createArgs,
                    NULL, &msc_obj->cbPrms);

                if (NULL == msc_obj->handle)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Fvid2_create failed\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
                else
                {
                    Fvid2Frame_init(&msc_obj->inFrm);
                    Fvid2Frame_init(&msc_obj->outFrm);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        msc_prms = &msc_obj->msc_prms;

        Vhwa_m2mMscParamsInit(msc_prms);

        tivxVpacMscScaleSetFmt(&msc_prms->inFmt, in_img_desc);

        if ((vx_df_image)VX_DF_IMAGE_NV12 != out_img_desc->format)
        {
            /* Only luma only mode, if the output is not NV12,
             * even if the input format is NV12. */
            msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        }

        if (1U == inst_obj->alloc_sc_fwd_dir)
        {
            idx = 0;
            msc_obj->sc_map_idx = idx;
        }
        else
        {
            idx = MSC_MAX_OUTPUT - 1U;
            msc_obj->sc_map_idx = idx;
        }
        fmt = &msc_prms->outFmt[idx];
        sc_cfg = &msc_prms->mscCfg.scCfg[idx];

        tivxVpacMscScaleSetScParams(
            sc_cfg, in_img_desc, out_img_desc, inst_obj->target_type);
        tivxVpacMscScaleSetFmt(fmt, out_img_desc);

        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
            msc_prms, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            status = (vx_status)VX_SUCCESS;

            /* Set up Frame List */
            msc_obj->inFrmList.numFrames = 1U;
            msc_obj->inFrmList.frames[0u] = &msc_obj->inFrm;
            msc_obj->outFrmList.numFrames = MSC_MAX_OUTPUT;
            msc_obj->outFrmList.frames[idx] = &msc_obj->outFrm;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, msc_obj,
            sizeof(tivxVpacMscHsgObj));
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
    tivxVpacMscHsgObj *msc_obj = NULL;
    tivxVpacMscScaleInstObj *inst_obj = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj) &&
            (sizeof(tivxVpacMscHsgObj) == size))
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
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
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
    int32_t                  gsize_value;
    uint32_t                 size;
    uint32_t                 plane_cnt;
    Fvid2_Frame             *frm = NULL;
    tivx_obj_desc_image_t   *in_img_desc;
    tivx_obj_desc_image_t   *out_img_desc;
    tivx_obj_desc_scalar_t  *gsize_desc;
    tivxVpacMscHsgObj       *msc_obj = NULL;
    Msc_Coeff               *coeffCfg = NULL;
    vx_enum                  interp_type;
    uint64_t                cur_time;
    tivxVpacMscScaleInstObj *inst_obj = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        in_img_desc = (tivx_obj_desc_image_t *)obj_desc
            [TIVX_KERNEL_HALFSCALE_GAUSSIAN_INPUT_IDX];
        out_img_desc = (tivx_obj_desc_image_t *)obj_desc
            [TIVX_KERNEL_HALFSCALE_GAUSSIAN_OUTPUT_IDX];
        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc
            [TIVX_KERNEL_HALFSCALE_GAUSSIAN_KERNEL_SIZE_IDX];
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL Params check failed\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
        }
        else if ((NULL == msc_obj) ||
            (sizeof(tivxVpacMscHsgObj) != size))
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
        }
        else
        {
            /* do nothing */
        }
    }

    /* First change the coefficient based on the gvalue */
    if ((vx_status)VX_SUCCESS == status)
    {
        coeffCfg = &msc_obj->coeffCfg;

        Msc_coeffInit(coeffCfg);

        if (TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_TARGET ==
                msc_obj->inst_obj->target_type)
        {
            gsize_value = gsize_desc->data.s32;

            /* Based on gsize value/kernel size, change the coefficients
             * for half scaling */
            if (1 == gsize_value)
            {
                coeffCfg->spCoeffSet[0U] = gVpacMscHsgSpCoeff[0U];
            }
            else if (3 == gsize_value)
            {
                coeffCfg->spCoeffSet[0U] = gVpacMscHsgSpCoeff[1U];
            }
            else /* 5 == gsize_value */
            {
                coeffCfg->spCoeffSet[0U] = gVpacMscHsgSpCoeff[2U];
            }
        }
        else
        {
            interp_type = gsize_desc->data.enm;

            if ((vx_enum)VX_INTERPOLATION_BILINEAR == interp_type)
            {
                coeffCfg->mpCoeffSet[0U] = gVpacMscScaleMpCoeff[0U];
            }
            else /* Nearest Neighbor coefficients */
            {
                coeffCfg->mpCoeffSet[0U] = gVpacMscScaleMpCoeff[1U];
            }
        }

        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            coeffCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create coefficients\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        frm = &msc_obj->inFrm;
        for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
        {
            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                (int32_t)in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
        }

        frm = &msc_obj->outFrm;
        for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
        {
            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                out_img_desc->mem_ptr[plane_cnt].shared_ptr,
                (int32_t)out_img_desc->mem_ptr[plane_cnt].mem_heap_region);
        }

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit MSC Request*/
        fvid2_status = Fvid2_processRequest(msc_obj->handle, &msc_obj->inFrmList,
            &msc_obj->outFrmList, FVID2_TIMEOUT_FOREVER);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Wait for Frame Completion */
        tivxEventWait(msc_obj->wait_for_compl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        fvid2_status = Fvid2_getProcessedRequest(msc_obj->handle,
            &msc_obj->inFrmList, &msc_obj->outFrmList, 0);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        inst_obj = msc_obj->inst_obj;

        if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format)
        {
            size = in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y + \
                   in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y/2;
        }
        else
        {
            size = in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y;
        }

        appPerfStatsHwaUpdateLoad(inst_obj->hwa_perf_id,
            (uint32_t)cur_time,
            size /* pixels processed */
            );
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacMscHsgObj *tivxVpacMscScaleAllocObject(tivxVpacMscScaleInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacMscHsgObj *msc_obj = NULL;

    /* Lock instance semaphore */
    SemaphoreP_pend(instObj->lock, SemaphoreP_WAIT_FOREVER);

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->msc_obj[cnt].isAlloc)
        {
            msc_obj = &instObj->msc_obj[cnt];
            memset(msc_obj, 0x0, sizeof(tivxVpacMscHsgObj));
            instObj->msc_obj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance semaphore */
    SemaphoreP_post(instObj->lock);

    return (msc_obj);
}

static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscHsgObj *msc_obj)
{
    uint32_t cnt;

    /* Lock instance semaphore */
    SemaphoreP_pend(instObj->lock, SemaphoreP_WAIT_FOREVER);

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    {
        if (msc_obj == &instObj->msc_obj[cnt])
        {
            msc_obj->isAlloc = 0U;
            break;
        }
    }

    /* Release instance semaphore */
    SemaphoreP_post(instObj->lock);
}

static void tivxVpacMscScaleSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
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
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
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
    const tivx_obj_desc_image_t *in_img_desc, const tivx_obj_desc_image_t *out_img_desc,
    uint32_t target_type)
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

        if (TIVX_VPAC_MSC_HALF_SCALE_GAUSSIAN_TARGET == target_type)
        {
            /* For Gaussian Half Scale, only single phase coefficients are used. */
            sc_cfg->filtMode = MSC_FILTER_MODE_SINGLE_PHASE;
            sc_cfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            sc_cfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            sc_cfg->coeffShift = MSC_COEFF_SHIFT_8;
        }
        else
        {
            /* For Scale, multi phase coefficients are used. */
            sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
            sc_cfg->phaseMode = MSC_PHASE_MODE_64PHASE;
            sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            sc_cfg->coeffShift = MSC_COEFF_SHIFT_8;
        }
    }
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacMscScaleFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacMscHsgObj *msc_obj = (tivxVpacMscHsgObj *)appData;

    if (NULL != msc_obj)
    {
        tivxEventPost(msc_obj->wait_for_compl);
    }

    return FVID2_SOK;
}


