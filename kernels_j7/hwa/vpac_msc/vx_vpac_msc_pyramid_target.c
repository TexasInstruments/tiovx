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

/* There are two target kernels supported by this implemenation,
 * one is MSC Pyramid node
 * two is OpenVX Gaussian Pyramid node
 */
#define TIVX_VPAC_MSC_NUM_INST                  (VHWA_M2M_MSC_MAX_INST * 2u)

#define TIVX_VPAC_MSC_PMD_START_IDX             (0u)
#define TIVX_VPAC_MSC_G_PMG_START_IDX           (2u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct tivxVpacMscPmdInstObj_t tivxVpacMscPmdInstObj;

typedef struct
{
    /*!< Index of the input from which this pyramid is generated
     *   For the 0th Octave, it will be generated from the source image.
     *   This is used only if the multiple octaves are required
     *   This typically points to the last output from the
     *   previeus pyramid set. */
    uint32_t                input_idx;
    /*!< Index from which the Pyramid level starts in the vx_pyramid object */
    uint32_t                out_start_idx;
    /*!< Number of pyramid levels within this pyramid set */
    uint32_t                num_levels;
    /*!< This maps output index to scaler index */
    uint32_t                sc_map_idx[MSC_MAX_OUTPUT];
} tivxVpacMscPmdSubSetInfo;

typedef struct
{
    /*! Flag to indicate if it msc object is allocated or not */
    uint32_t                 isAlloc;

    /*! MSC Driver Create Arguments */
    Vhwa_M2mMscCreatePrms    createArgs;
    /*! MSC Driver parameters, locally storing it for each subset pyramid */
    Vhwa_M2mMscParams        msc_prms[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];
    /*! MSC Driver Coefficients */
    Msc_Coeff                coeffCfg;
    /*! Driver Handle */
    Fvid2_Handle             handle;
    /*! Mutext to wait for completion */
    tivx_event               wait_for_compl;

    /*! Input Fvid2 Frame List */
    Fvid2_FrameList          inFrmList;
    /*! Input Fvid2 Frame List */
    Fvid2_FrameList          outFrmList;
    /*! Input Fvid2 Frame */
    Fvid2_Frame              inFrm;
    /*! List of FVID2 Output Frames */
    Fvid2_Frame              outFrm[MSC_MAX_OUTPUT];
    /*! FVID2 Callback parameters */
    Fvid2_CbParams           cbPrms;

    /*! Locally storing pointer to instance object for easy access. */
    tivxVpacMscPmdInstObj   *inst_obj;

    /*! Input image descriptor, locally storing for easy access. */
    tivx_obj_desc_image_t   *in_img_desc;
    /*! Output image descriptor, locally storing for easy access. */
    tivx_obj_desc_image_t   *out_img_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];


    /*! Pyramid Subset information, it stores information like
     *  number of levels in this subset, input start index, output
     *  start index etc.. */
    tivxVpacMscPmdSubSetInfo ss_info[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];

    /*! Number of pyramid subsets, essentially valid entries
     *  in ss_info array */
    uint32_t                 num_pmd_subsets;

    /*! Total number of pyramid levels, locally storing for easy access. */
    uint32_t                 num_pmd_levels;
} tivxVpacMscPmdObj;


struct tivxVpacMscPmdInstObj_t
{
    /*! Mutex protecting msc objects allocation and free */
    tivx_mutex              lock;

    /*! MSC Objects */
    tivxVpacMscPmdObj       msc_obj[VHWA_M2M_MSC_MAX_HANDLES];

    /*! Locally storing target kernel, this is used to identify
     *  the target kernel instance */
    tivx_target_kernel      target_kernel;

    /*! Flag to indicate whether to allocate scaler from top ie from
     *  instance 0 or from bottom instance ie instance 9.
     *  Instance0 of the target kernels allocates scaler in forward direction,
     *  starting from scaler0, whereas Instance1 of the target kernels
     *  allocate in backward direction from scaler9. */
    uint32_t                alloc_sc_fwd_dir;

    /*! Instance ID of the MSC driver */
    uint32_t                msc_drv_inst_id;

    /*! HWA Performance ID */
    app_perf_hwa_id_t       hwa_perf_id;
} ;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* OPENVX Callback functions */
static vx_status VX_CALLBACK tivxVpacMscPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

/* Local Functions */
static tivxVpacMscPmdObj *tivxVpacMscPmdAllocObject(tivxVpacMscPmdInstObj *instObj);
static void tivxVpacMscPmdFreeObject(tivxVpacMscPmdInstObj *instObj,
    tivxVpacMscPmdObj *msc_obj);
static void tivxVpacMscPmdSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc,
    uint32_t level,
    tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdObj *msc_obj, tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdSetMscParams(tivxVpacMscPmdObj *msc_obj,
    tivxVpacMscPmdSubSetInfo *ss_info, uint32_t num_oct,
    tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms);

/* Control Command Implementation */
static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);

/* Driver Callback */
int32_t tivxVpacMscPmdFrameComplCb(Fvid2_Handle handle, void *appData);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

tivxVpacMscPmdInstObj gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_NUM_INST];

static int32_t gmsc_32_phase_gaussian_filter[] =
{
    #include "../host/msc_32_phase_gaussian_filter.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscGaussianPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    char                    target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                 self_cpu;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_G_PMG_START_IDX;
    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if ((self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0))
#else
    if (((vx_enum)TIVX_CPU_ID_MCU2_0 == self_cpu)
#if defined(SOC_J784S4)
        || ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
#endif
        )
#endif
    {
        /* Reset all values to 0 */
        memset(&gTivxVpacMscPmdInstObj[inst_start], 0x0,
            sizeof(tivxVpacMscPmdInstObj) * VHWA_M2M_MSC_MAX_INST);

        for (cnt = 0u; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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
                                (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                                target_name,
                                tivxVpacMscPmdProcess,
                                tivxVpacMscPmdCreate,
                                tivxVpacMscPmdDelete,
                                tivxVpacMscPmdControl,
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
        }

        /* Clean up allocated resources */
        if ((vx_status)VX_SUCCESS != status)
        {
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                inst_obj = &gTivxVpacMscPmdInstObj[cnt];
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

void tivxRemoveTargetKernelVpacMscGaussianPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_G_PMG_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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


void tivxAddTargetKernelVpacMscPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    char                    target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                 self_cpu;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_PMD_START_IDX;
    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if ((self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0))
#else
    if (((vx_enum)TIVX_CPU_ID_MCU2_0 == self_cpu)
#if defined(SOC_J784S4)
        || ((vx_enum)TIVX_CPU_ID_MCU4_0 == self_cpu)
#endif
        )
#endif
    {
        /* Reset all values to 0 */
        memset(&gTivxVpacMscPmdInstObj[inst_start], 0x0,
            sizeof(tivxVpacMscPmdInstObj) * VHWA_M2M_MSC_MAX_INST);

        for (cnt = 0u; (cnt < VHWA_M2M_MSC_MAX_INST) && (status == (vx_status)VX_SUCCESS); cnt ++)
        {
            inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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

            inst_obj->target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
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
        }

        /* Clean up allocated resources */
        if ((vx_status)VX_SUCCESS != status)
        {
            for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
            {
                inst_obj = &gTivxVpacMscPmdInstObj[cnt];
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

void tivxRemoveTargetKernelVpacMscPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_PMD_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
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

static vx_status VX_CALLBACK tivxVpacMscPmdCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    vx_uint32                cnt;
    tivxVpacMscPmdObj       *msc_obj = NULL;
    tivx_obj_desc_image_t   *in_img_desc = NULL;
    tivx_obj_desc_pyramid_t *out_pmd_desc = {NULL};
    tivxVpacMscPmdInstObj   *inst_obj = NULL;
    tivx_target_kernel       target_kernel = NULL;

    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

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
            for (cnt = 0u; cnt < TIVX_VPAC_MSC_NUM_INST; cnt ++)
            {
                if (target_kernel == gTivxVpacMscPmdInstObj[cnt].target_kernel)
                {
                    inst_obj = &gTivxVpacMscPmdInstObj[cnt];
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
        msc_obj = tivxVpacMscPmdAllocObject(inst_obj);
        if (NULL != msc_obj)
        {
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            /* Initialize Msc object */
            msc_obj->inst_obj = inst_obj;
            msc_obj->in_img_desc = in_img_desc;
            msc_obj->num_pmd_levels = out_pmd_desc->num_levels;

            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)msc_obj->out_img_desc,
                out_pmd_desc->num_levels);

            for (cnt = 0U; cnt < out_pmd_desc->num_levels; cnt ++)
            {
                if (NULL == msc_obj->out_img_desc[cnt])
                {
                    VX_PRINT(VX_ZONE_ERROR, "Null Output Descriptor\n");
                    status = (vx_status)VX_FAILURE;
                    break;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Based on input and number of output images,
                 * create and initialize msc driver parametes */
                status = tivxVpacMscPmdCalcSubSetInfo(msc_obj, kernel);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Handle Object, increase VHWA_M2M_MSC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        Vhwa_M2mMscCreatePrmsInit(&msc_obj->createArgs);

        status = tivxEventCreate(&msc_obj->wait_for_compl);
        if ((vx_status)VX_SUCCESS == status)
        {
            msc_obj->cbPrms.cbFxn   = tivxVpacMscPmdFrameComplCb;
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
                for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
                {
                    Fvid2Frame_init(&msc_obj->outFrm[cnt]);
                }
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
        }
    }

    /* Setting coefficients and calling IOCTL */
    if ((vx_status)VX_SUCCESS == status)
    {
        Msc_Coeff  *coeffCfg;
        int32_t  single_phase[TIVX_VPAC_MSC_MAX_SP_COEFF_SET][TIVX_VPAC_MSC_MAX_TAP];

        coeffCfg = &msc_obj->coeffCfg;

        Msc_coeffInit(coeffCfg);

        cnt = 0;
        single_phase[0][cnt ++] = 0;
        single_phase[0][cnt ++] = 0;
        single_phase[0][cnt ++] = 256;
        single_phase[0][cnt ++] = 0;
        single_phase[0][cnt ++] = 0;
        cnt = 0;
        single_phase[1][cnt ++] = 16;
        single_phase[1][cnt ++] = 64;
        single_phase[1][cnt ++] = 96;
        single_phase[1][cnt ++] = 64;
        single_phase[1][cnt ++] = 16;

        for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
        {
            coeffCfg->spCoeffSet[cnt] = &single_phase[cnt][0u];
        }

        /* Coefficients for Gaussian filter */
        for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
        {
            coeffCfg->mpCoeffSet[cnt] = &gmsc_32_phase_gaussian_filter[0];
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
        /* Calling Set_Params for all Octaves, to verify the parameters
         * for all octaves */
        for (cnt = 0U; cnt < msc_obj->num_pmd_subsets; cnt ++)
        {
            fvid2_status = Fvid2_control(msc_obj->handle,
                VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                &msc_obj->msc_prms[cnt], NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            status = (vx_status)VX_SUCCESS;

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
            sizeof(tivxVpacMscPmdObj));
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

            tivxVpacMscPmdFreeObject(inst_obj, msc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacMscPmdObj *msc_obj = NULL;
    tivxVpacMscPmdInstObj *inst_obj = NULL;

    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj) &&
            (sizeof(tivxVpacMscPmdObj) == size))
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

            tivxVpacMscPmdFreeObject(inst_obj, msc_obj);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    int32_t                   fvid2_status = FVID2_SOK;
    uint32_t                  size;
    uint32_t                  out_cnt;
    uint32_t                  plane_cnt;
    uint32_t                  in_idx, sc_idx, out_img_idx;
    uint32_t                  oct_cnt;
    Fvid2_Frame              *frm = NULL;
    tivx_obj_desc_image_t    *in_img_desc;
    tivx_obj_desc_image_t    *img_desc;
    tivx_obj_desc_pyramid_t  *out_pmd_desc;
    tivxVpacMscPmdObj        *msc_obj = NULL;
    Fvid2_FrameList          *inFrmList;
    Fvid2_FrameList          *outFrmList;
    tivxVpacMscPmdSubSetInfo *ss_info;
    uint64_t                 cur_time;
    tivxVpacMscPmdInstObj    *inst_obj = NULL;

    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL Params check failed\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&msc_obj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj) &&
            (sizeof(tivxVpacMscPmdObj) == size))
        {
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)msc_obj->out_img_desc,
                out_pmd_desc->num_levels);

            for (out_cnt = 0U; out_cnt < out_pmd_desc->num_levels; out_cnt ++)
            {
                if (NULL == msc_obj->out_img_desc[out_cnt])
                {
                    VX_PRINT(VX_ZONE_ERROR, "Null Descriptor for Output\n");
                    status = (vx_status)VX_FAILURE;
                    break;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        inFrmList = &msc_obj->inFrmList;
        outFrmList = &msc_obj->outFrmList;

        /* Number of input frames is fixed to 1 */
        inFrmList->numFrames = 1U;

        cur_time = tivxPlatformGetTimeInUsecs();

        for (oct_cnt = 0u; (oct_cnt < msc_obj->num_pmd_subsets) && (status == (vx_status)VX_SUCCESS); oct_cnt ++)
        {
            ss_info = &msc_obj->ss_info[oct_cnt];

            /* MSC Parameters requires to be set in the driver,
             * if the number of octaves are more than 1.
             * If it is only one, params are already set in the driver as
             * part of create, so no need to set again. */
            if (1u < msc_obj->num_pmd_subsets)
            {
                fvid2_status = Fvid2_control(msc_obj->handle,
                    VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                    &msc_obj->msc_prms[oct_cnt], NULL);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to set params\n");
                    status = (vx_status)VX_FAILURE;
                }
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                frm = &msc_obj->inFrm;
                /* For the first octave, input is from the actual input image */
                if (0u == oct_cnt)
                {
                    for (plane_cnt = 0u; plane_cnt < in_img_desc->planes;
                            plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
                    }
                }
                else
                {
                    /* For the rest octaves, Use the last output
                     * from the previous octave, as an input */
                    in_idx = ss_info->input_idx;
                    img_desc = msc_obj->out_img_desc[in_idx];
                    for (plane_cnt = 0u; plane_cnt < img_desc->planes;
                            plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            img_desc->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)img_desc->mem_ptr[plane_cnt].mem_heap_region);
                    }
                }

                outFrmList->numFrames = MSC_MAX_OUTPUT;
                out_img_idx = ss_info->out_start_idx;
                for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
                {
                    img_desc = msc_obj->out_img_desc[out_img_idx];
                    sc_idx = ss_info->sc_map_idx[out_cnt];
                    frm = &msc_obj->outFrm[sc_idx];

                    for (plane_cnt = 0u; plane_cnt < img_desc->planes;
                            plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            img_desc->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)img_desc->mem_ptr[plane_cnt].
                            mem_heap_region);
                    }
                    out_img_idx ++;
                }

                /* Submit MSC Request*/
                fvid2_status = Fvid2_processRequest(msc_obj->handle, inFrmList,
                    outFrmList, FVID2_TIMEOUT_FOREVER);
                if (FVID2_SOK != fvid2_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
                    status = (vx_status)VX_FAILURE;
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    /* Wait for Frame Completion */
                    tivxEventWait(msc_obj->wait_for_compl, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

                    fvid2_status = Fvid2_getProcessedRequest(msc_obj->handle,
                        inFrmList, outFrmList, 0);
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
            }
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

static vx_status VX_CALLBACK tivxVpacMscPmdControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    uint32_t             size;
    tivxVpacMscPmdObj *msc_obj = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&msc_obj, &size);

    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == msc_obj) ||
        (sizeof(tivxVpacMscPmdObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
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
                status = tivxVpacMscPmdSetCoeffsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetInputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetOutputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Command Id\n");
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

static tivxVpacMscPmdObj *tivxVpacMscPmdAllocObject(
    tivxVpacMscPmdInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacMscPmdObj *msc_obj = NULL;

    /* Lock instance mutex */
    tivxMutexLock(instObj->lock);

    for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
    {
        if (0U == instObj->msc_obj[cnt].isAlloc)
        {
            msc_obj = &instObj->msc_obj[cnt];
            memset(msc_obj, 0x0, sizeof(tivxVpacMscPmdObj));
            instObj->msc_obj[cnt].isAlloc = 1U;
            break;
        }
    }

    /* Release instance mutex */
    tivxMutexUnlock(instObj->lock);

    return (msc_obj);
}

static void tivxVpacMscPmdFreeObject(tivxVpacMscPmdInstObj *instObj,
    tivxVpacMscPmdObj *msc_obj)
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

static void tivxVpacMscPmdSetFmt(Fvid2_Format *fmt,
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

static void tivxVpacMscPmdSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc,
    uint32_t level,
    tivx_target_kernel_instance kernel)
{
    tivx_target_kernel       target_kernel = NULL;

    target_kernel = tivxTargetKernelInstanceGetKernel(kernel);

    if ((NULL != in_img_desc) && (NULL != out_img_desc))
    {
        vx_float32 temp;
        sc_cfg->enable = TRUE;
        sc_cfg->filtMode = MSC_FILTER_MODE_SINGLE_PHASE;

        sc_cfg->outWidth = out_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->outHeight = out_img_desc->imagepatch_addr[0].dim_y;
        sc_cfg->inRoi.cropStartX = 0u;
        sc_cfg->inRoi.cropStartY = 0u;
        sc_cfg->inRoi.cropWidth = in_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y;
        temp = (((((vx_float32)sc_cfg->inRoi.cropWidth/(vx_float32)sc_cfg->outWidth) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->horzAccInit = (uint32_t)temp;
        temp = (((((vx_float32)sc_cfg->inRoi.cropHeight/(vx_float32)sc_cfg->outHeight) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->vertAccInit = (uint32_t)temp;
        sc_cfg->hsSpCoeffSel = 1;
        sc_cfg->vsSpCoeffSel = 1;

        /* Note: in the case that it is using a Gaussian pyramid, select the first set of coefficients for first level */
        if ( (0U == level) &&
               ((gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX].target_kernel == target_kernel) ||
                (gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX+1U].target_kernel == target_kernel)) )
        {
            sc_cfg->hsSpCoeffSel = 0;
            sc_cfg->vsSpCoeffSel = 0;
        }
        else
        {
            if(!((sc_cfg->outWidth == sc_cfg->inRoi.cropWidth) ||
                ((sc_cfg->outWidth*2u) == sc_cfg->inRoi.cropWidth) ||
                ((sc_cfg->outWidth*4u) == sc_cfg->inRoi.cropWidth)) )
            {
                sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
                sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;
                sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            }

            if(!((sc_cfg->outHeight == sc_cfg->inRoi.cropHeight) ||
                ((sc_cfg->outHeight*2u) == sc_cfg->inRoi.cropHeight) ||
                ((sc_cfg->outHeight*4u) == sc_cfg->inRoi.cropHeight)) )
            {
                sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
                sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;
                sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            }
        }
    }
}

static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdObj *msc_obj, tivx_target_kernel_instance kernel)
{
    vx_status                   status = (vx_status)VX_SUCCESS;
    uint32_t                    cnt;
    uint32_t                    num_pmd_levels;
    uint32_t                    num_subsets;
    tivx_obj_desc_image_t      *in_img_desc;
    tivx_obj_desc_image_t      *out_img_desc;
    tivxVpacMscPmdSubSetInfo   *ss_info;
    uint32_t                    max_ds_factor = TIVX_VPAC_MSC_MAX_DS_FACTOR;
    tivx_target_kernel          target_kernel;

    target_kernel = tivxTargetKernelInstanceGetKernel(kernel);

    /* For vxGaussianPyramid, the Khronos conformance tests for random input fail unless
     * max_ds_factor is set to 2
     */
    if ((gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX].target_kernel == target_kernel) ||
        (gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX+1U].target_kernel == target_kernel))
    {
        max_ds_factor = 2;
    }

    /* TODO:
     * This is temporarily hard coding to 2 in order to pass existing test that is based
     * on Khronos test.  However, when we relax this, it is better to put to 4 for
     * speed performance improvements.
     */
    max_ds_factor = 2;

    if (NULL != msc_obj)
    {
        num_subsets = 0U;
        in_img_desc = msc_obj->in_img_desc;
        out_img_desc = msc_obj->out_img_desc[0u];
        num_pmd_levels = msc_obj->num_pmd_levels;

        ss_info = &msc_obj->ss_info[0U];

        /* Atleast, for the first level,
         * the scaling factor cannot be less than 1/max_ds_factor */
        if (((in_img_desc->imagepatch_addr[0u].dim_x /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_x) ||
            ((in_img_desc->imagepatch_addr[0u].dim_y /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_y))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Scaling Factor\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            ss_info->input_idx = 0u;
            ss_info->out_start_idx = 0u;
            ss_info->num_levels = 0u;

            /* Atleast, one subset is required */
            num_subsets ++;

            for (cnt = 0u; (cnt < num_pmd_levels) && (status == (vx_status)VX_SUCCESS); cnt ++)
            {
                out_img_desc = msc_obj->out_img_desc[cnt];

                /* Need to change pyramid subset,
                 * if input to output ratio is more than max_ds_factor
                 */
                if (((in_img_desc->imagepatch_addr[0].dim_x /
                        max_ds_factor) >
                        out_img_desc->imagepatch_addr[0].dim_x) ||
                    ((in_img_desc->imagepatch_addr[0].dim_y /
                        max_ds_factor) >
                        out_img_desc->imagepatch_addr[0].dim_y))
                {
                    /* Get the next pyramid subset */
                    ss_info = &msc_obj->ss_info[num_subsets];

                    /* Input image for the this pyramid subset is the
                     * last output from previous pyramid subset */
                    in_img_desc = msc_obj->out_img_desc[cnt - 1u];

                    /* Initialize input and output indices */
                    ss_info->input_idx = cnt - 1u;
                    ss_info->out_start_idx = cnt;

                    /* Atleast, this level is required for
                     * this pyramid subset */
                    ss_info->num_levels = 1u;

                    num_subsets ++;
                    if (TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO
                        <= num_subsets)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Pyramid Subsets required are more than TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    ss_info->num_levels ++;
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    if (MSC_MAX_OUTPUT < ss_info->num_levels)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Max 10 outputs supported in subset\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }
        }

        msc_obj->num_pmd_subsets = num_subsets;
    }

    if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj))
    {
        /* Now, set the scaler information for each pyramid subset */
        for (cnt = 0u; cnt < msc_obj->num_pmd_subsets; cnt ++)
        {
            tivxVpacMscPmdSetMscParams(msc_obj, &msc_obj->ss_info[cnt], cnt, kernel);
        }
    }

    return (status);
}

static void tivxVpacMscPmdSetMscParams(tivxVpacMscPmdObj *msc_obj,
    tivxVpacMscPmdSubSetInfo *ss_info, uint32_t num_oct,
    tivx_target_kernel_instance kernel)
{
    uint32_t                 out_cnt;
    uint32_t                 idx;
    uint32_t                 out_start_idx;
    Vhwa_M2mMscParams       *msc_prms = NULL;
    tivx_obj_desc_image_t   *in_img_desc;
    tivx_obj_desc_image_t   *img_desc;

    msc_prms = &msc_obj->msc_prms[num_oct];
    out_start_idx = ss_info->out_start_idx;

    if (0U == num_oct)
    {
        in_img_desc = msc_obj->in_img_desc;
    }
    else
    {
        in_img_desc = msc_obj->out_img_desc[ss_info->input_idx];
    }

    img_desc = msc_obj->out_img_desc[out_start_idx];

    /* Initialize MSC Parameters with the default configuration */
    Vhwa_m2mMscParamsInit(msc_prms);

    /* Set the input format */
    tivxVpacMscPmdSetFmt(&msc_prms->inFmt, in_img_desc);

    if ((vx_df_image)VX_DF_IMAGE_NV12 != img_desc->format)
    {
        /* Only luma only mode, if the output is not NV12,
         * even if the input format is NV12. */
        msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
    }

    for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
    {
        if (1U == msc_obj->inst_obj->alloc_sc_fwd_dir)
        {
            idx = out_cnt;
        }
        else
        {
            idx = MSC_MAX_OUTPUT - 1U - out_cnt;
        }

        ss_info->sc_map_idx[out_cnt] = idx;

        tivxVpacMscPmdSetScParams(&msc_prms->mscCfg.scCfg[idx],
            in_img_desc, msc_obj->out_img_desc[out_start_idx], out_start_idx, kernel);

        tivxVpacMscPmdSetFmt(&msc_prms->outFmt[idx],
            msc_obj->out_img_desc[out_start_idx]);

        out_start_idx ++;
    }
}

static void tivxVpacMscPmdCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms)
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
                    VX_PRINT(VX_ZONE_ERROR, "Incorrect multi-phase horz coeff, defaulting to set 0\n");
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
                    VX_PRINT(VX_ZONE_ERROR, "Incorrect multi-phase horz coeff, defaulting to set 0\n");
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

static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          cnt;
    tivx_vpac_msc_coefficients_t     *coeffs = NULL;
    void                             *target_ptr;
    Msc_Coeff                        *coeffCfg = NULL;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

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
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            coeffCfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create coefficients\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, ss_idx, sc_idx;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;
    tivxVpacMscPmdSubSetInfo         *ss_info = NULL;
    Msc_ScConfig                     *sc_cfg = NULL;

    ss_info = &msc_obj->ss_info[0u];
    for (cnt = 0u; cnt < msc_obj->num_pmd_levels; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            if (sizeof(tivx_vpac_msc_output_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;

                /* Except the last subset pyramid, all subset
                 * pyramid have same number of levels */
                ss_idx = cnt / ss_info->num_levels;

                /* Scaler index within subset pyramid can be calculated
                 * by just modulo operation. */
                sc_idx = cnt % ss_info->num_levels;

                /* Now map scaler index to scaler config index */
                sc_idx = ss_info->sc_map_idx[sc_idx];

                sc_cfg = &msc_obj->msc_prms[ss_idx].mscCfg.scCfg[sc_idx];

                tivxVpacMscPmdCopyOutPrmsToScCfg(sc_cfg, out_prms);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Output Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Null User Data Object\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }


        if ((vx_status)VX_SUCCESS == status)
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Set Output Params\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    void                             *target_ptr;
    Vhwa_M2mMscParams                *msc_prms = NULL;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_input_params_t) ==
                usr_data_obj->mem_size)
        {
            in_prms = (tivx_vpac_msc_input_params_t *)target_ptr;

            for (cnt = 0u; cnt < msc_obj->num_pmd_subsets; cnt ++)
            {
                msc_prms = &msc_obj->msc_prms[cnt];

                msc_prms->enableLineSkip = in_prms->src_ln_inc_2;

                switch (in_prms->kern_sz)
                {
                    case 3:
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_3TAPS;
                        break;
                    case 4:
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_4TAPS;
                        break;
                    case 5:
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_5TAPS;
                        break;
                    default:
                        VX_PRINT(VX_ZONE_ERROR, "Invalid Kernel Size\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        break;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Set Input Params\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}


/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacMscPmdFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacMscPmdObj *msc_obj = (tivxVpacMscPmdObj *)appData;

    if (NULL != msc_obj)
    {
        tivxEventPost(msc_obj->wait_for_compl);
    }

    return FVID2_SOK;
}


