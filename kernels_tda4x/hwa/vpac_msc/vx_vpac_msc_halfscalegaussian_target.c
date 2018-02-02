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
#include "TI/tda4x.h"
#include "VX/vx.h"
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_halfscale_gaussian.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "scaler_core.h"

static tivx_target_kernel vx_halfscale_gaussian_target_kernel = NULL;

typedef struct
{
    uint16_t *src16;
    uint16_t *dst16;
    uint32_t buffer_size_in;
    uint32_t buffer_size_out;

    Scaler_Config mmr;
} tivxHalfScaleGaussianParams;

static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxKernelHalfScaleGaussianFreeMem(tivxHalfScaleGaussianParams *prms);


static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxHalfScaleGaussianParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *gsize_desc;
    uint32_t size;
    unsigned short *imgInput[2];
    unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            if ((NULL == prms) || (size != sizeof(tivxHalfScaleGaussianParams)))
            {
                status = VX_FAILURE;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);

        /* C-model supports only 12-bit in uint16_t container
         * So we may need to translate.  In HW, VPAC_LSE does this
         */
        lse_reformat_in(src, prms->src16);

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
    }

    if (VX_SUCCESS == status)
    {
        int32_t gsize_value = gsize_desc->data.s32;

        imgInput[0] = prms->src16;
        imgOutput[0] = prms->dst16;

        prms->mmr.G_inWidth[0] = src->imagepatch_addr[0].dim_x;
        prms->mmr.G_inHeight[0] = src->imagepatch_addr[0].dim_y;

        prms->mmr.unitParams[0].filter_mode = 0;

        prms->mmr.unitParams[0].sp_hs_coef_sel = 0;
        prms->mmr.unitParams[0].sp_vs_coef_sel = 0;

        prms->mmr.unitParams[0].threadMap = 0;
        prms->mmr.unitParams[0].coefShift = 8;
        prms->mmr.unitParams[0].outWidth = dst->imagepatch_addr[0].dim_x;
        prms->mmr.unitParams[0].outHeight = dst->imagepatch_addr[0].dim_y;
        prms->mmr.unitParams[0].hzScale = 4096*2;
        prms->mmr.unitParams[0].vtScale = 4096*2;//.f/pmd->scale;

        prms->mmr.cfg_Kernel[0].Sz_height = 5;
        prms->mmr.cfg_Kernel[0].Tpad_sz = 2;
        prms->mmr.cfg_Kernel[0].Bpad_sz = 2;

        if (1 == gsize_value)
        {
            prms->mmr.coef_sp[0][0] = 0;
            prms->mmr.coef_sp[0][1] = 0;
            prms->mmr.coef_sp[0][2] = 256;
            prms->mmr.coef_sp[0][3] = 0;
            prms->mmr.coef_sp[0][4] = 0;
        }
        else if (3 == gsize_value)
        {
            prms->mmr.coef_sp[0][0] = 0;
            prms->mmr.coef_sp[0][1] = 64;
            prms->mmr.coef_sp[0][2] = 128;
            prms->mmr.coef_sp[0][3] = 64;
            prms->mmr.coef_sp[0][4] = 0;
        }
        else /* 5 == gsize_value */
        {
            prms->mmr.coef_sp[0][0] = 16;
            prms->mmr.coef_sp[0][1] = 64;
            prms->mmr.coef_sp[0][2] = 96;
            prms->mmr.coef_sp[0][3] = 64;
            prms->mmr.coef_sp[0][4] = 16;
        }

        scaler_top_processing(imgInput, imgOutput, &prms->mmr);
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t stub;

        stub.valid_roi.start_x = 0;
        stub.valid_roi.start_y = 0;

        /* Reformat output */
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        lse_reformat_out(&stub, dst, prms->dst16, 12);

        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *imgIn, *imgOut;
    tivxHalfScaleGaussianParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        imgIn = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        imgOut = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];

        prms = tivxMemAlloc(sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxHalfScaleGaussianParams));

            prms->buffer_size_in = imgIn->imagepatch_addr[0].dim_x *
                                   imgIn->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size_in, TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->buffer_size_out = imgOut->imagepatch_addr[0].dim_x *
                                        imgOut->imagepatch_addr[0].dim_y * 2;

                prms->dst16 = tivxMemAlloc(prms->buffer_size_out, TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxHalfScaleGaussianParams));
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxHalfScaleGaussianParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHalfScaleGaussianParams) == size))
        {
            tivxKernelHalfScaleGaussianFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelHalfScaleGaussianControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelVpacMscHalfScaleGaussian(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
            TIVX_TARGET_MAX_NAME);

        vx_halfscale_gaussian_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HALFSCALE_GAUSSIAN,
            target_name,
            tivxKernelHalfScaleGaussianProcess,
            tivxKernelHalfScaleGaussianCreate,
            tivxKernelHalfScaleGaussianDelete,
            tivxKernelHalfScaleGaussianControl,
            NULL);
    }
}


void tivxRemoveTargetKernelVpacMscHalfScaleGaussian(void)
{
    tivxRemoveTargetKernel(vx_halfscale_gaussian_target_kernel);
    vx_halfscale_gaussian_target_kernel = NULL;
}

static void tivxKernelHalfScaleGaussianFreeMem(tivxHalfScaleGaussianParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size_in, TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }
        if (NULL != prms->dst16)
        {
            tivxMemFree(prms->dst16, prms->buffer_size_out, TIVX_MEM_EXTERNAL);
            prms->dst16 = NULL;
        }

        tivxMemFree(prms, sizeof(tivxHalfScaleGaussianParams), TIVX_MEM_EXTERNAL);
    }
}
