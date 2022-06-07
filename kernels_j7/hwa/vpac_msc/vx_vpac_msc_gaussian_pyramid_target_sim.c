/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_gaussian_pyramid.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_msc_priv.h"
#include "vx_kernels_hwa_target.h"
#include "scaler_core.h"

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_gaussian_pyramid_target_kernel[4] = {NULL};

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint16_t *src16;
    uint16_t *dst16[2];
    uint32_t buffer_size;

    msc_config config;
} tivxGassPyrmdParams;

static vx_status VX_CALLBACK tivxKernelGsnPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelGsnPmdCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelGsnPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxKernelGsnPmdFreeMem(tivxGassPyrmdParams *prms);


static vx_status VX_CALLBACK tivxKernelGsnPmdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxGassPyrmdParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_pyramid_t *pmd;
    uint32_t size, levels;
    unsigned short *imgInput[2];
    unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_GAUSSIAN_PYRAMID_INPUT_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[TIVX_KERNEL_GAUSSIAN_PYRAMID_GAUSSIAN_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS == status)
        {
            if ((NULL != prms) && (size == sizeof(tivxGassPyrmdParams)))
            {
                tivxGetObjDescList(pmd->obj_desc_id,
                    (tivx_obj_desc_t **)prms->img_obj_desc, pmd->num_levels);

                for (levels = 0U; levels < pmd->num_levels; levels ++)
                {
                    if (NULL == prms->img_obj_desc[levels])
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
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        /* C-model supports only 12-bit in uint16_t container
         * So we may need to translate.  In HW, VPAC_LSE does this
         */
        lse_reformat_in(src, src_target_ptr, prms->src16, 0, 0);

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        imgInput[0] = prms->src16;
        imgOutput[0] = prms->dst16[0];
        imgOutput[1] = prms->dst16[1];

        /* Process levels 0 and 1 */
        prms->config.settings.G_inWidth[0] = src->imagepatch_addr[0].dim_x;
        prms->config.settings.G_inHeight[0] = src->imagepatch_addr[0].dim_y;

        /* 1x output for level 0 of pyramid */
        prms->config.settings.coef_sp[0][0] = 0;
        prms->config.settings.coef_sp[0][1] = 0;
        prms->config.settings.coef_sp[0][2] = 256;
        prms->config.settings.coef_sp[0][3] = 0;
        prms->config.settings.coef_sp[0][4] = 0;

        prms->config.settings.unitParams[0].threadMap = 0;
        prms->config.settings.unitParams[0].coefShift = 8;
        prms->config.settings.unitParams[0].outWidth = prms->img_obj_desc[0]->imagepatch_addr[0].dim_x;
        prms->config.settings.unitParams[0].outHeight = prms->img_obj_desc[0]->imagepatch_addr[0].dim_y;
        prms->config.settings.unitParams[0].hzScale = 4096;
        prms->config.settings.unitParams[0].vtScale = 4096;

        prms->config.settings.cfg_Kernel[0].Sz_height = 5;
        prms->config.settings.cfg_Kernel[0].Tpad_sz = 2;
        prms->config.settings.cfg_Kernel[0].Bpad_sz = 2;

        if(pmd->scale == 0.5f)
        {
            prms->config.settings.unitParams[1].filter_mode = 0;

            prms->config.settings.coef_sp[1][0] = 16;
            prms->config.settings.coef_sp[1][1] = 64;
            prms->config.settings.coef_sp[1][2] = 96;
            prms->config.settings.coef_sp[1][3] = 64;
            prms->config.settings.coef_sp[1][4] = 16;

            prms->config.settings.unitParams[1].sp_hs_coef_sel = 1;
            prms->config.settings.unitParams[1].sp_vs_coef_sel = 1;
        }
        else
        /* This does not pass the OpenVX conformance test for ORB scale */
        /* Reason: OpenVX defines nearest neighbor scale on fixed gaussian pre-filter
         *         Our hardware uses polyphase, best we can do is bilinear interpolation,
         *         or ceil() */
        {
            int32_t i;

            prms->config.settings.unitParams[1].filter_mode = 1;
            prms->config.settings.unitParams[1].initPhaseX = 4096.f*(1.f-pmd->scale);
            prms->config.settings.unitParams[1].initPhaseY = 4096.f*(1.f-pmd->scale);

            for(i=0; i<32; i++)
            {
                prms->config.settings.coef_mp[0].matrix[i][0] = 16;
                prms->config.settings.coef_mp[0].matrix[i][1] = 64;
                prms->config.settings.coef_mp[0].matrix[i][2] = 96;
                prms->config.settings.coef_mp[0].matrix[i][3] = 64;
                prms->config.settings.coef_mp[0].matrix[i][4] = 16;
            }

            for(i=0; i<32; i++)
            {
                prms->config.settings.coef_mp[1].matrix[i][0] = 16;
                prms->config.settings.coef_mp[1].matrix[i][1] = 64;
                prms->config.settings.coef_mp[1].matrix[i][2] = 96;
                prms->config.settings.coef_mp[1].matrix[i][3] = 64;
                prms->config.settings.coef_mp[1].matrix[i][4] = 16;
            }
        }

        prms->config.settings.unitParams[1].threadMap = 0;
        prms->config.settings.unitParams[1].coefShift = 8;
        prms->config.settings.unitParams[1].outWidth = prms->img_obj_desc[1]->imagepatch_addr[0].dim_x;
        prms->config.settings.unitParams[1].outHeight = prms->img_obj_desc[1]->imagepatch_addr[0].dim_y;
        prms->config.settings.unitParams[1].hzScale = 4096.f/pmd->scale;
        prms->config.settings.unitParams[1].vtScale = 4096.f/pmd->scale;

        prms->config.settings.cfg_Kernel[0].Sz_height = 5;
        prms->config.settings.cfg_Kernel[0].Tpad_sz = 2;
        prms->config.settings.cfg_Kernel[0].Bpad_sz = 2;

#ifdef VLAB_HWA

        prms->config.magic = 0xC0DEFACE;
        prms->config.buffer[0]  = imgInput[0];
        prms->config.buffer[4]  = imgOutput[0];
        prms->config.buffer[6]  = imgOutput[1];

        status = vlab_hwa_process(VPAC_MSC_BASE_ADDRESS, "VPAC_MSC_GAUSSIAN_PYRAMID", sizeof(msc_config), &prms->config);

#else

        scaler_top_processing(imgInput, imgOutput, &prms->config.settings);

#endif
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t bufCnt = 1;
        tivx_obj_desc_image_t stub;
        void *dst_target_ptr;

        stub.valid_roi.start_x = 0;
        stub.valid_roi.start_y = 0;

        /* Reformat levels 0 and 1 */
        dst = prms->img_obj_desc[0];
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        lse_reformat_out(&stub, dst, dst_target_ptr, prms->dst16[0], 12, 0);

        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        dst = prms->img_obj_desc[1];
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        lse_reformat_out(&stub, dst, dst_target_ptr, prms->dst16[1], 12, 0);

        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        imgOutput[0] = NULL;

        /* Process remaining levels one at a time */
        for (levels = 2; (levels < pmd->num_levels) && ((vx_status)VX_SUCCESS == status);
                levels ++)
        {
            src = prms->img_obj_desc[levels - 1U];
            dst = prms->img_obj_desc[levels];

            imgInput[0] = prms->dst16[bufCnt];
            imgOutput[1] = prms->dst16[bufCnt^1];

            prms->config.settings.G_inWidth[0] = src->imagepatch_addr[0].dim_x;
            prms->config.settings.G_inHeight[0] = src->imagepatch_addr[0].dim_y;
            prms->config.settings.unitParams[1].outWidth = dst->imagepatch_addr[0].dim_x;
            prms->config.settings.unitParams[1].outHeight = dst->imagepatch_addr[0].dim_y;

#ifdef VLAB_HWA

            prms->config.magic = 0xC0DEFACE;
            prms->config.buffer[0]  = imgInput[0];
            prms->config.buffer[4]  = imgOutput[0];
            prms->config.buffer[6]  = imgOutput[1];

            status = vlab_hwa_process(VPAC_MSC_BASE_ADDRESS, "VPAC_MSC_GAUSSIAN_PYRAMID", sizeof(msc_config), &prms->config);

#else

            scaler_top_processing(imgInput, imgOutput, &prms->config.settings);

#endif
            dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
            tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            lse_reformat_out(&stub, dst, dst_target_ptr, prms->dst16[bufCnt^1], 12, 0);

            tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            bufCnt ^= 1;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelGsnPmdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *img;
    tivxGassPyrmdParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_GAUSSIAN_PYRAMID_INPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxGassPyrmdParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxGassPyrmdParams));

            prms->buffer_size = img->imagepatch_addr[0].dim_x *
                                img->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->dst16[0] = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16[0])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->dst16[1] = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16[1])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxGassPyrmdParams));
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelGsnPmdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_GAUSSIAN_PYRAMID_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxGassPyrmdParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxGassPyrmdParams) == size))
        {
            tivxKernelGsnPmdFreeMem(prms);
        }
    }

    return status;
}

/* ToDo: this function is defined in multiple places */
void tivxAddTargetKernelVpacMscGaussianPyramid(void)
{
    vx_status status;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

#ifdef SOC_AM62A
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
#else
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
#endif
    {
        strncpy(target_name, TIVX_TARGET_VPAC_MSC1, TIVX_TARGET_MAX_NAME);
        vx_gaussian_pyramid_target_kernel[0] = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
            target_name,
            tivxKernelGsnPmdProcess,
            tivxKernelGsnPmdCreate,
            tivxKernelGsnPmdDelete,
            NULL,
            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC_MSC2, TIVX_TARGET_MAX_NAME);
        vx_gaussian_pyramid_target_kernel[1] = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
            target_name,
            tivxKernelGsnPmdProcess,
            tivxKernelGsnPmdCreate,
            tivxKernelGsnPmdDelete,
            NULL,
            NULL);

        status = (vx_status)VX_SUCCESS;
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_MSC1, TIVX_TARGET_MAX_NAME);
        vx_gaussian_pyramid_target_kernel[2] = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
            target_name,
            tivxKernelGsnPmdProcess,
            tivxKernelGsnPmdCreate,
            tivxKernelGsnPmdDelete,
            NULL,
            NULL);

        strncpy(target_name, TIVX_TARGET_VPAC2_MSC2, TIVX_TARGET_MAX_NAME);
        vx_gaussian_pyramid_target_kernel[3] = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
            target_name,
            tivxKernelGsnPmdProcess,
            tivxKernelGsnPmdCreate,
            tivxKernelGsnPmdDelete,
            NULL,
            NULL);

        status = (vx_status)VX_SUCCESS;
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
    }
}


void tivxRemoveTargetKernelVpacMscGaussianPyramid(void)
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[0]);
        vx_gaussian_pyramid_target_kernel[0] = NULL;

        tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[1]);
        vx_gaussian_pyramid_target_kernel[1] = NULL;
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[2]);
        vx_gaussian_pyramid_target_kernel[2] = NULL;

        tivxRemoveTargetKernel(vx_gaussian_pyramid_target_kernel[3]);
        vx_gaussian_pyramid_target_kernel[3] = NULL;
    }
    #endif
}

static void tivxKernelGsnPmdFreeMem(tivxGassPyrmdParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }
        if (NULL != prms->dst16[0])
        {
            tivxMemFree(prms->dst16[0], prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dst16[0] = NULL;
        }
        if (NULL != prms->dst16[1])
        {
            tivxMemFree(prms->dst16[1], prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dst16[1] = NULL;
        }

        tivxMemFree(prms, sizeof(tivxGassPyrmdParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}
