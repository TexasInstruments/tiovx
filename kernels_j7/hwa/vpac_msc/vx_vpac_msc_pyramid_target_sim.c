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
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_scale.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_msc_priv.h"
#include "vx_kernels_hwa_target.h"
#include "scaler_core.h"
#include "tivx_kernel_vpac_msc.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    /*!< Index of the input from which this pyramid is generated
     *   For the 0th pyramid subset, it will be generated from the source image.
     *   This is used only if the multiple pyramid subsets are required
     *   This typically points to the last output from the
     *   previeus pyramid set. */
    uint32_t                input_idx;
    /*!< Index from which the Pyramid level starts in the vx_pyramid object */
    uint32_t                out_start_idx;
    /*!< Number of pyramid levels within this pyramid set */
    uint32_t                num_levels;
} tivxVpacMscPmdSubSetInfo;

typedef struct
{
    tivx_obj_desc_image_t      *in_img_desc;
    tivx_obj_desc_image_t      *out_img_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];


    tivxVpacMscPmdSubSetInfo    ss_info[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];
    uint32_t                    num_pmd_subsets;

    uint32_t                    num_pmd_levels;

    msc_config                  config;

    uint16_t                   *src16;
    uint16_t                   *dst16[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint32_t                    buffer_size_in;
    uint32_t                    buffer_size_out[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];

    Scaler_params               unitParams[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
} tivxVpacMscPmdParams;

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

static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);

/* Local Functions */
static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdParams *prms, tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdInitCoeff(Scaler_Config *settings);
static void tivxVpacMscPmdFreeMem(tivxVpacMscPmdParams *prms);
static void tivxVpacMscInitScalerUnitParams(tivxVpacMscPmdParams *prms, tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdMaskLSBs(uint16_t *ptr16, uint32_t w, uint32_t h);
static void tivxVpacMscPmdSetLevelPhase(tivxVpacMscPmdParams *prms, tivx_obj_desc_image_t *in_img_desc, uint32_t level);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#define NUM_MSC_TARGET_KERNEL_INSTANCES 4

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_vpac_msc_pyramid_target_kernel[NUM_MSC_TARGET_KERNEL_INSTANCES] = {NULL};

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_gaussian_pyramid_target_kernel[NUM_MSC_TARGET_KERNEL_INSTANCES] = {NULL};

static uint32_t gmsc_32_phase_gaussian_filter[] =
{
    #include "../host/msc_32_phase_gaussian_filter.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscPyramid(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME);

        vx_vpac_msc_pyramid_target_kernel[0]  = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_msc_pyramid_target_kernel[1]  = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_msc_pyramid_target_kernel[2]  = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_msc_pyramid_target_kernel[3]  = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacMscPyramid()
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        if (NULL != vx_vpac_msc_pyramid_target_kernel[0])
        {
            tivxRemoveTargetKernel(vx_vpac_msc_pyramid_target_kernel[0]);
            vx_vpac_msc_pyramid_target_kernel[0] = NULL;
        }
        if (NULL != vx_vpac_msc_pyramid_target_kernel[1])
        {
            tivxRemoveTargetKernel(vx_vpac_msc_pyramid_target_kernel[1]);
            vx_vpac_msc_pyramid_target_kernel[1] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        if (NULL != vx_vpac_msc_pyramid_target_kernel[2])
        {
            tivxRemoveTargetKernel(vx_vpac_msc_pyramid_target_kernel[2]);
            vx_vpac_msc_pyramid_target_kernel[2] = NULL;
        }
        if (NULL != vx_vpac_msc_pyramid_target_kernel[3])
        {
            tivxRemoveTargetKernel(vx_vpac_msc_pyramid_target_kernel[3]);
            vx_vpac_msc_pyramid_target_kernel[3] = NULL;
        }
    }
    #endif
}

void tivxAddTargetKernelVpacMscGaussianPyramid(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME);
        vx_gaussian_pyramid_target_kernel[0] = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
            TIVX_TARGET_MAX_NAME);

        vx_gaussian_pyramid_target_kernel[1] = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
            TIVX_TARGET_MAX_NAME);

        vx_gaussian_pyramid_target_kernel[2] = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
            TIVX_TARGET_MAX_NAME);

        vx_gaussian_pyramid_target_kernel[3] = tivxAddTargetKernel(
                            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacMscGaussianPyramid(void)
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        if (NULL != vx_gaussian_pyramid_target_kernel[0])
        {
            tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[0]);
            vx_gaussian_pyramid_target_kernel[0] = NULL;
        }
        if (NULL != vx_gaussian_pyramid_target_kernel[1])
        {
            tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[1]);
            vx_gaussian_pyramid_target_kernel[1] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        if (NULL != vx_gaussian_pyramid_target_kernel[2])
        {
            tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[2]);
            vx_gaussian_pyramid_target_kernel[2] = NULL;
        }
        if (NULL != vx_gaussian_pyramid_target_kernel[3])
        {
            tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[3]);
            vx_gaussian_pyramid_target_kernel[3] = NULL;
        }
    }
    #endif
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
    vx_uint32                cnt;
    tivxVpacMscPmdParams    *prms = NULL;
    tivx_obj_desc_image_t   *in_img_desc = NULL;
    tivx_obj_desc_image_t   *out_img_desc = NULL;
    tivx_obj_desc_pyramid_t *out_pmd_desc = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        prms = tivxMemAlloc(sizeof(tivxVpacMscPmdParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset (prms, 0x0, sizeof(tivxVpacMscPmdParams));

            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            prms->in_img_desc = in_img_desc;
            prms->num_pmd_levels = out_pmd_desc->num_levels;

            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)prms->out_img_desc,
                out_pmd_desc->num_levels);

            for (cnt = 0U; cnt < out_pmd_desc->num_levels; cnt ++)
            {
                if (NULL == prms->out_img_desc[cnt])
                {
                    status = (vx_status)VX_FAILURE;
                    break;
                }
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Based on input and number of output images,
             * create and initialize msc driver parametes */
            status = tivxVpacMscPmdCalcSubSetInfo(prms, kernel);
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->buffer_size_in = in_img_desc->imagepatch_addr[0].dim_x *
                                   in_img_desc->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                VX_PRINT(VX_ZONE_ERROR, "Input Buffer Alloc Error\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            for (cnt = 0u; cnt < out_pmd_desc->num_levels; cnt ++)
            {
                out_img_desc = prms->out_img_desc[cnt];

                prms->buffer_size_out[cnt] =
                    out_img_desc->imagepatch_addr[0].dim_x *
                    out_img_desc->imagepatch_addr[0].dim_y * 2;

                prms->dst16[cnt] = tivxMemAlloc(prms->buffer_size_out[cnt],
                    (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16[cnt])
                {
                    VX_PRINT(VX_ZONE_ERROR, "Output%d Buffer Alloc Error\n",
                        cnt);
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxVpacMscPmdInitCoeff(&prms->config.settings);

            tivxVpacMscInitScalerUnitParams(prms, kernel);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxSetTargetKernelInstanceContext(kernel, prms,
            sizeof(tivxVpacMscPmdParams));
    }
    else
    {
        tivxVpacMscPmdFreeMem(prms);
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status              status = (vx_status)VX_SUCCESS;
    uint32_t               size;
    tivxVpacMscPmdParams  *prms = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacMscPmdParams) == size))
        {
            tivxVpacMscPmdFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    uint32_t                  size;
    uint32_t                  out_cnt;
    uint32_t                  in_idx, out_img_idx;
    uint32_t                  oct_cnt;
    uint32_t                  iw, ih;
    uint32_t                  ow, oh;
    uint32_t                  hzScale, vtScale;
    tivx_obj_desc_image_t    *in_img_desc = NULL;
    tivx_obj_desc_image_t    *out_img_desc = NULL;
    tivx_obj_desc_pyramid_t  *out_pmd_desc = NULL;
    tivxVpacMscPmdParams     *prms = NULL;
    tivxVpacMscPmdSubSetInfo *ss_info;
    void                     *target_ptr;
    uint16_t                 *src_ptr;
    uint16_t                 *dst_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint32_t                  pmd_level_cnt;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacMscPmdParams) == size))
        {
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)prms->out_img_desc,
                out_pmd_desc->num_levels);

            for (out_cnt = 0U; out_cnt < out_pmd_desc->num_levels; out_cnt ++)
            {
                if (NULL == prms->out_img_desc[out_cnt])
                {
                    status = (vx_status)VX_FAILURE;
                    break;
                }
            }
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        pmd_level_cnt = 0u;

        for (oct_cnt = 0u; oct_cnt < prms->num_pmd_subsets; oct_cnt ++)
        {
            ss_info = &prms->ss_info[oct_cnt];

            /* For the first pyramid subset, input is from
             * the actual input image */
            if (0u == oct_cnt)
            {
                iw = in_img_desc->imagepatch_addr[0].dim_x;
                ih = in_img_desc->imagepatch_addr[0].dim_y;

                prms->config.settings.G_inWidth[0] = iw;
                prms->config.settings.G_inHeight[0] = ih;

                target_ptr = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, in_img_desc->mem_size[0],
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                /* C-model supports only 12-bit in uint16_t container
                 * So we may need to translate.  In HW, VPAC_LSE does this
                 */
                lse_reformat_in(in_img_desc, target_ptr, prms->src16, 0, 0);

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, in_img_desc->mem_size[0],
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                src_ptr = prms->src16;
            }
            else
            {
                /* For the rest pyramid subsets, Use the last output
                 * from the previous pmd subset, as an input */
                in_idx = ss_info->input_idx;

                /* Input is anyway from one of the previous output image
                 * and this does not overwrite input image descriptor. */
                out_img_desc = prms->out_img_desc[in_idx];

                iw = out_img_desc->imagepatch_addr[0u].dim_x;
                ih = out_img_desc->imagepatch_addr[0u].dim_y;

                prms->config.settings.G_inWidth[0u] = iw;
                prms->config.settings.G_inHeight[0u] = ih;

                src_ptr = prms->dst16[in_idx];

                /* Intermediate buffer between levels is 12 bit.  If output
                 * is 8 bit, then 4 LSBs should be set to 0 before processing
                 * next level */
                if (((vx_df_image)VX_DF_IMAGE_U8 == out_img_desc->format) ||
                    ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc->format))
                {
                    tivxVpacMscPmdMaskLSBs(src_ptr, iw, ih);
                }
            }

            out_img_idx = ss_info->out_start_idx;
            for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
            {
                out_img_desc = prms->out_img_desc[out_img_idx];

                ow = out_img_desc->imagepatch_addr[0].dim_x;
                oh = out_img_desc->imagepatch_addr[0].dim_y;
                hzScale = ((float)(4096*iw)/(float)ow) + 0.5f;
                vtScale = ((float)(4096*ih)/(float)oh) + 0.5f;

                memcpy(&prms->config.settings.unitParams[out_cnt],
                    &prms->unitParams[pmd_level_cnt], sizeof(Scaler_params));

                prms->config.settings.unitParams[out_cnt].outWidth =
                    out_img_desc->imagepatch_addr[0].dim_x;
                prms->config.settings.unitParams[out_cnt].outHeight =
                    out_img_desc->imagepatch_addr[0].dim_y;
                prms->config.settings.unitParams[out_cnt].hzScale = hzScale;
                prms->config.settings.unitParams[out_cnt].vtScale = vtScale;

                /* Control Command provides an interface for setting
                 * init phase information, but currently overriding it
                 * using this equation. */
                prms->config.settings.unitParams[out_cnt].initPhaseX =
                    (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
                prms->config.settings.unitParams[out_cnt].initPhaseY =
                    (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;

                dst_ptr[out_cnt] = prms->dst16[out_img_idx];

                out_img_idx ++;

                pmd_level_cnt ++;
            }

#ifdef VLAB_HWA

            prms->config.magic = 0xC0DEFACE;
            prms->config.buffer[0]  = src_ptr;
            for (out_cnt = 0; out_cnt < ss_info->num_levels; out_cnt ++)
            {
                prms->config.buffer[4 + (out_cnt * 2)]  = dst_ptr[out_cnt];
            }

            status = vlab_hwa_process(VPAC_MSC_BASE_ADDRESS,
                "VPAC_MSC_SCALE", sizeof(msc_config), &prms->config);
#else
            {
                unsigned short *imgInput[2];
                unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

                imgInput[0] = src_ptr;

                for (out_cnt = 0; out_cnt < ss_info->num_levels; out_cnt ++)
                {
                    imgOutput[out_cnt] = dst_ptr[out_cnt];
                }

                scaler_top_processing(imgInput, imgOutput, &prms->config.settings);

            }
#endif
            if ((vx_status)VX_SUCCESS == status)
            {
                tivx_obj_desc_image_t stub;

                stub.valid_roi.start_x = 0u;
                stub.valid_roi.start_y = 0u;

                out_img_idx = ss_info->out_start_idx;
                for (out_cnt = 0; out_cnt < ss_info->num_levels; out_cnt ++)
                {
                    out_img_desc = prms->out_img_desc[out_img_idx];

                    /* Reformat output */
                    target_ptr = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr[0]);
                    tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, out_img_desc->mem_size[0],
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                    lse_reformat_out(&stub, out_img_desc, target_ptr,
                        dst_ptr[out_cnt], 12, 0);

                    tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, out_img_desc->mem_size[0],
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

                    out_img_idx ++;
                }
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacMscPmdControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    uint32_t                      size;
    tivxVpacMscPmdParams         *prms = NULL;

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacMscPmdParams) == size))
        {
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_MSC_CMD_SET_COEFF:
            {
                status = tivxVpacMscPmdSetCoeffsCmd(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetInputParamsCmd(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetOutputParamsCmd(prms,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            default:
            {
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

static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdParams *prms, tivx_target_kernel_instance kernel)
{
    vx_status                   status = (vx_status)VX_SUCCESS;
    uint32_t                    cnt;
    uint32_t                    num_pmd_levels;
    uint32_t                    num_subsets;
    tivx_obj_desc_image_t      *in_img_desc;
    tivx_obj_desc_image_t      *out_img_desc;
    tivxVpacMscPmdSubSetInfo   *ss_info;
    uint32_t                    max_ds_factor = TIVX_VPAC_MSC_MAX_DS_FACTOR;
    vx_bool is_gaussian_pyramid_target_kernel = vx_false_e;
    int i;

    /* For vxGaussianPyramid, the Khronos conformance tests for random input fail unless
     * max_ds_factor is set to 2
     */
    for (i = 0u; i < NUM_MSC_TARGET_KERNEL_INSTANCES; i++)
    {
        if (vx_gaussian_pyramid_target_kernel[i] == tivxTargetKernelInstanceGetKernel(kernel))
        {
            is_gaussian_pyramid_target_kernel = vx_true_e;
        }
    }
    if (vx_true_e == is_gaussian_pyramid_target_kernel)
    {
        max_ds_factor = 2;
    }

    /* TODO:
     * This is temporarily hard coding to 2 in order to pass existing test that is based
     * on Khronos test.  However, when we relax this, it is better to put to 4 for
     * speed performance improvements.
     */
    max_ds_factor = 2;

    if (NULL != prms)
    {
        num_subsets = 0U;
        in_img_desc = prms->in_img_desc;
        out_img_desc = prms->out_img_desc[0u];
        num_pmd_levels = prms->num_pmd_levels;

        ss_info = &prms->ss_info[0U];

        /* Atleast, for the first level,
         * the scaling factor cannot be less than 1/max_ds_factor */
        if (((in_img_desc->imagepatch_addr[0u].dim_x /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_x) ||
            ((in_img_desc->imagepatch_addr[0u].dim_y /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_y))
        {
            VX_PRINT(VX_ZONE_ERROR, "Scale should not be less than 1/%d\n", max_ds_factor);
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            ss_info->input_idx = 0u;
            ss_info->out_start_idx = 0u;
            ss_info->num_levels = 0u;

            /* Atleast, one subset is required */
            num_subsets ++;

            for (cnt = 0u; cnt < num_pmd_levels; cnt ++)
            {
                out_img_desc = prms->out_img_desc[cnt];

                tivxVpacMscPmdSetLevelPhase(prms, in_img_desc, cnt);

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
                    ss_info = &prms->ss_info[num_subsets];

                    /* Input image for the this pyramid subset is the
                     * last output from previous pyramid subset */
                    in_img_desc = prms->out_img_desc[cnt - 1u];

                    /* Initialize input and output indices */
                    ss_info->input_idx = cnt - 1u;
                    ss_info->out_start_idx = cnt;

                    /* Atleast, this level is required for
                     * this pyramid subset */
                    ss_info->num_levels = 1u;

                    num_subsets ++;
                    if (num_subsets >=
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Pyramid Subsets required are more than TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        break;
                    }
                }
                else
                {
                    ss_info->num_levels ++;
                }

                if (ss_info->num_levels > TIVX_VPAC_MSC_MAX_OUTPUT)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Max %d outputs supported in subset\n", TIVX_VPAC_MSC_MAX_OUTPUT);
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
                }
            }
        }

        prms->num_pmd_subsets = num_subsets;
    }

    return (status);
}

static void tivxVpacMscPmdSetLevelPhase(tivxVpacMscPmdParams *prms, tivx_obj_desc_image_t *in_img_desc, uint32_t level)
{
    uint32_t iw, ih, ow, oh;

    iw = in_img_desc->imagepatch_addr[0].dim_x;
    ih = in_img_desc->imagepatch_addr[0].dim_y;
    ow = prms->out_img_desc[level]->imagepatch_addr[0].dim_x;
    oh = prms->out_img_desc[level]->imagepatch_addr[0].dim_y;

    if(!((ow == iw) ||
        ((ow*2u) == iw) ||
        ((ow*4u) == iw)) ||
       !((oh == ih) ||
        ((oh*2u) == ih) ||
        ((oh*4u) == ih)) )
    {
        prms->unitParams[level].filter_mode = 1;
        prms->unitParams[level].phase_mode = 1;
    }
    else
    {
        prms->unitParams[level].filter_mode = 0;
        prms->unitParams[level].phase_mode = 0;
    }
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    void                         *target_ptr;
    tivx_vpac_msc_coefficients_t *coeffs;
    uint32_t                      i, j, idx;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_coefficients_t) ==
                usr_data_obj->mem_size)
        {
            coeffs = (tivx_vpac_msc_coefficients_t *)target_ptr;
            for (i = 0; i < TIVX_VPAC_MSC_MAX_MP_COEFF_SET; i ++)
            {
                idx = 0;
                for (j = 0; j < TIVX_VPAC_MSC_32_PHASE_COEFF; j ++)
                {
                    prms->config.settings.coef_mp[i].matrix[j][0] =
                        coeffs->multi_phase[i][idx ++];
                    prms->config.settings.coef_mp[i].matrix[j][1] =
                        coeffs->multi_phase[i][idx ++];
                    prms->config.settings.coef_mp[i].matrix[j][2] =
                        coeffs->multi_phase[i][idx ++];
                    prms->config.settings.coef_mp[i].matrix[j][3] =
                        coeffs->multi_phase[i][idx ++];
                    prms->config.settings.coef_mp[i].matrix[j][4] =
                        coeffs->multi_phase[i][idx ++];
                }
            }

            for (i = 0; i < TIVX_VPAC_MSC_MAX_SP_COEFF_SET; i ++)
            {
                for (j = 0; j < TIVX_VPAC_MSC_MAX_TAP; j ++)
                {
                    prms->config.settings.coef_sp[i][j] =
                        coeffs->single_phase[i][j];
                }
            }
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_input_params_t) ==
                usr_data_obj->mem_size)
        {
            in_prms = (tivx_vpac_msc_input_params_t *)target_ptr;

            switch (in_prms->kern_sz)
            {
                case 3:
                    prms->config.settings.cfg_Kernel[0].Sz_height = 3;
                    prms->config.settings.cfg_Kernel[0].Tpad_sz = 1;
                    prms->config.settings.cfg_Kernel[0].Bpad_sz = 1;
                    break;
                case 4:
                    prms->config.settings.cfg_Kernel[0].Sz_height = 4;
                    prms->config.settings.cfg_Kernel[0].Tpad_sz = 1;
                    prms->config.settings.cfg_Kernel[0].Bpad_sz = 2;
                    break;
                case 5:
                    prms->config.settings.cfg_Kernel[0].Sz_height = 5;
                    prms->config.settings.cfg_Kernel[0].Tpad_sz = 2;
                    prms->config.settings.cfg_Kernel[0].Bpad_sz = 2;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Kernel Size\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
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

    return (status);
}

static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;

    for (cnt = 0u; cnt < prms->num_pmd_levels; cnt ++)
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

                prms->unitParams[cnt].signedData = out_prms->signed_data;
                prms->unitParams[cnt].filter_mode = out_prms->filter_mode;
                prms->unitParams[cnt].coefShift = out_prms->coef_shift;
                prms->unitParams[cnt].satMode = out_prms->saturation_mode;
                prms->unitParams[cnt].x_offset = out_prms->offset_x;
                prms->unitParams[cnt].y_offset = out_prms->offset_y;
                prms->unitParams[cnt].sp_hs_coef_src = out_prms->single_phase.horz_coef_src;
                prms->unitParams[cnt].sp_hs_coef_sel = out_prms->single_phase.horz_coef_sel;
                prms->unitParams[cnt].sp_vs_coef_src = out_prms->single_phase.vert_coef_src;
                prms->unitParams[cnt].sp_vs_coef_src = out_prms->single_phase.vert_coef_src;

                prms->unitParams[cnt].phase_mode = out_prms->multi_phase.phase_mode;
                prms->unitParams[cnt].hs_coef_sel = out_prms->multi_phase.horz_coef_sel;
                prms->unitParams[cnt].vs_coef_sel = out_prms->multi_phase.vert_coef_sel;
                prms->unitParams[cnt].initPhaseX = out_prms->multi_phase.init_phase_x;
                prms->unitParams[cnt].initPhaseY = out_prms->multi_phase.init_phase_y;
            }
            else
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }
        else
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }


        if ((vx_status)VX_SUCCESS == status)
        {
            break;
        }
    }

    return (status);
}



/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */


static void tivxVpacMscPmdInitCoeff(Scaler_Config *settings)
{
    uint32_t i,j,k;

    /* coefficients 1x output */
    settings->coef_sp[0u][0u] = 0;
    settings->coef_sp[0u][1u] = 0;
    settings->coef_sp[0u][2u] = 256;
    settings->coef_sp[0u][3u] = 0;
    settings->coef_sp[0u][4u] = 0;

    /* coefficients 0.5x output */
    settings->coef_sp[1u][0u] = 16;
    settings->coef_sp[1u][1u] = 64;
    settings->coef_sp[1u][2u] = 96;
    settings->coef_sp[1u][3u] = 64;
    settings->coef_sp[1u][4u] = 16;

    /* Coefficients for Gaussian filter */
    for(k = 0; k < 4; k++)
    {
        for(i=0; i<TIVX_VPAC_MSC_32_PHASE_COEFF; i++)
        {
            for(j=0; j<5; j++)
            {
                settings->coef_mp[k].matrix[i][j] = gmsc_32_phase_gaussian_filter[(i*5)+j];
            }
        }
    }
}

static void tivxVpacMscPmdFreeMem(tivxVpacMscPmdParams *prms)
{
    uint32_t cnt;

    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }

        for (cnt = 0u; cnt < TIVX_PYRAMID_MAX_LEVEL_OBJECTS; cnt ++)
        {
            if ((NULL != prms->dst16[cnt]) &&
                (0U != prms->buffer_size_out[cnt]))
            {
                tivxMemFree(prms->dst16[cnt], prms->buffer_size_out[cnt],
                    (vx_enum)TIVX_MEM_EXTERNAL);
                prms->dst16[cnt] = NULL;
            }
        }

        tivxMemFree(prms, sizeof(tivxVpacMscPmdParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static void tivxVpacMscInitScalerUnitParams(tivxVpacMscPmdParams *prms, tivx_target_kernel_instance kernel)
{
    int i;
    vx_bool is_gaussian_pyramid_target_kernel = vx_false_e;

    /* Be default, 5-tap filter */
    prms->config.settings.cfg_Kernel[0].Sz_height = 5;
    prms->config.settings.cfg_Kernel[0].Tpad_sz = 2;
    prms->config.settings.cfg_Kernel[0].Bpad_sz = 2;

    for (i = 0u; i < NUM_MSC_TARGET_KERNEL_INSTANCES; i++)
    {
        if (vx_gaussian_pyramid_target_kernel[i] == tivxTargetKernelInstanceGetKernel(kernel))
        {
            is_gaussian_pyramid_target_kernel = vx_true_e;
        }
    }

    for (i = 0u; i < prms->num_pmd_levels; i ++)
    {
        prms->unitParams[i].threadMap = 0;
        prms->unitParams[i].coefShift = 8;

        prms->unitParams[i].hs_coef_sel = 0;
        prms->unitParams[i].vs_coef_sel = 0;

        prms->unitParams[i].x_offset = 0;
        prms->unitParams[i].y_offset = 0;

        prms->unitParams[i].sp_hs_coef_sel = 1;
        prms->unitParams[i].sp_vs_coef_sel = 1;

        /* Note: in the case that it is using a Gaussian pyramid, select the first set of coefficients for first level */
        /* For vxGaussianPyramid, level 0 of the output pyramid is the same
         * size as this input (always).  Therefore, set the coefficients for
         * level 0 to be set to unity.
         *
         * TODO: for tivxVpacMscPyramid, normal use case is to downsample,
         * but it could be option in future to have as same size as input.
         * In this case, a DMA would be more optimal
         */
        if ( (0U == i) &&
             (vx_true_e == is_gaussian_pyramid_target_kernel) )
        {
            prms->unitParams[0].sp_hs_coef_sel = 0;
            prms->unitParams[0].sp_vs_coef_sel = 0;
        }
    }
}

static void tivxVpacMscPmdMaskLSBs(uint16_t *ptr16, uint32_t w, uint32_t h)
{
    uint32_t i, j;

    for(j = 0; j < h; j++)
    {
        for(i=0; i < w; i++)
        {
            /* Mask lower 4 bits */
            ptr16[(j*w)+i] &= 0xFF0;
        }
    }
}
