/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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
#include "tivx_kernel_vpac_nf_generic.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_nf_priv.h"
#include "vx_kernels_hwa_target.h"
#include <TI_BilateralFilter_cn.h>

typedef struct
{
    /* Pointers to src and dest */
    uint16_t *src16;
    uint16_t *dst16;
    uint32_t buffer_size;

    bilateralFilter_mmr_t mmr;
    bilateralFilter_debug_t debug;

} tivxVpacNfGenericParams;

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_vpac_nf_generic_target_kernel[2] = {NULL};

static vx_status VX_CALLBACK tivxVpacNfGenericProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfGenericCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfGenericDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacNfGenericFreeMem(tivxVpacNfGenericParams *prms);

#ifndef VLAB_HWA

/* Static hardware configuration from J7 */
static bilateralFilter_algParams_t    algParams[] =
{
    {
        8,          /* rangeLutQuantSz */
        8,          /* lutValueBitWidth */
        24,         /* recipValueBitWidth */
        0,          /* interpolation */
        4,          /* recipLutQuantSz */
        512         /* recipLutQuantCutoff */
    }
};

#endif

static vx_status VX_CALLBACK tivxVpacNfGenericProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    tivxVpacNfGenericParams *prms = NULL;
    tivx_obj_desc_convolution_t *conv;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *src, *dst;
        int32_t k;
        int16_t temp_lut[25];
        int16_t i_lut[24];
        int16_t *pConv;
        void *src_target_ptr;
        void *dst_target_ptr;
        void *conv_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_INPUT_IDX];
        conv = (tivx_obj_desc_convolution_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_CONV_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_OUTPUT_IDX];


        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxVpacNfGenericParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
            dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
            conv_target_ptr = tivxMemShared2TargetPtr(&conv->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(conv_target_ptr, conv->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            /* C-model supports only 12-bit in uint16_t container
             * So we may need to translate.  In HW, NF_LSE does this
             */
            lse_reformat_in(src, src_target_ptr, prms->src16, 0, 0);

            pConv = conv_target_ptr;

            /* Centers the given matrix in the 5x5 for the c-model */
            {
                int32_t m;

                for (m = -2; m < 3; m++)
                {
                    for (k = -2; k < 3; k++)
                    {
                        if ( ((int32_t)m < (int32_t)(-(conv->rows/2))) || ((int32_t)m > (int32_t)(conv->rows/2)))
                        {
                            /* Force to zero */
                            temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                        }
                        else if ( ((int32_t)k < (int32_t)(-(conv->columns/2))) || ((int32_t)k > (int32_t)(conv->columns/2)))
                        {
                            /* Force to zero */
                            temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                        }
                        else
                        {
                            temp_lut[((m + 2) * 5) + (k + 2)] = pConv[((m + (conv->rows/2)) * conv->columns) +
                                                                       (k + (conv->columns/2))];
                        }
                    }
                }
            }

            /* Since it is convolution, flip the matrix (decide later to do this or not) */
            for (k = 0; k < 12; k++) {
                i_lut[k] = temp_lut[25 - 1 - k];
            }
            for (k = 13; k < 25; k++) {
                i_lut[k - 1] = temp_lut[25 - 1 - k];
            }

            prms->mmr.centerPixelWeight = temp_lut[12];
            prms->mmr.lut = (uint16_t*)&i_lut;

#ifdef VLAB_HWA

            status = vlab_hwa_process(VPAC_NF_BASE_ADDRESS, "VPAC_NF_GENERIC", sizeof(bilateralFilter_mmr_t), &prms->mmr);

#else
            status = bilateral_hw(&algParams[0], &prms->mmr, &prms->debug);
            if (0 != status)
            {
                status = (vx_status)VX_FAILURE;
            }
#endif
        }
        if ((vx_status)VX_SUCCESS == status)
        {

            lse_reformat_out(src, dst, dst_target_ptr, prms->dst16, 12, 0);

            tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(conv_target_ptr, conv->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfGenericCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *src;
        tivxVpacNfGenericParams *prms = NULL;

        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_VPAC_NF_GENERIC_INPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxVpacNfGenericParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVpacNfGenericParams));

            prms->buffer_size = src->imagepatch_addr[0].dim_x *
                                src->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->dst16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tivx_vpac_nf_common_params_t *params;
                tivx_obj_desc_user_data_object_t *params_array;
                void *params_array_target_ptr;

                params_array = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_CONFIGURATION_IDX];

                params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                params = (tivx_vpac_nf_common_params_t *)params_array_target_ptr;

                prms->mmr.filterMode = FILTERMODE_GENERIC;
                prms->mmr.width = src->imagepatch_addr[0].dim_x;
                prms->mmr.height = src->imagepatch_addr[0].dim_y;
                prms->mmr.input = prms->src16;
                prms->mmr.output = prms->dst16;

                prms->mmr.inputPlaneInterleaving = params->input_interleaved;
                prms->mmr.shift = params->output_downshift;
                prms->mmr.offset = params->output_offset;
                prms->mmr.subRangeBits = 0;
                prms->mmr.adaptiveMode = 0;
                prms->mmr.subTableSelect = 0;
                prms->mmr.outputPixelSkip = params->output_pixel_skip;
                prms->mmr.outputPixelSkipOdd = params->output_pixel_skip_odd;
                prms->mmr.kern_ln_offset = params->kern_ln_offset;
                prms->mmr.kern_sz_height = params->kern_sz_height;
                prms->mmr.src_ln_inc_2 = params->src_ln_inc_2;

                prms->debug.fileTapName = NULL;
                prms->debug.tapMask = 0;
                prms->debug.recipFileName = NULL;
                prms->debug.replicateH_off = 0;
                prms->debug.replicateV_off = 0;

                tivxCheckStatus(&status, tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxVpacNfGenericParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacNfGenericFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfGenericDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacNfGenericParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacNfGenericParams) == size))
        {
            tivxVpacNfGenericFreeMem(prms);
        }
    }

    return status;
}

void tivxAddTargetKernelVpacNfGeneric(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_NF, TIVX_TARGET_MAX_NAME);
        vx_vpac_nf_generic_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_NF_GENERIC_NAME,
                            target_name,
                            tivxVpacNfGenericProcess,
                            tivxVpacNfGenericCreate,
                            tivxVpacNfGenericDelete,
                            NULL,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_NF, TIVX_TARGET_MAX_NAME);
        vx_vpac_nf_generic_target_kernel[1] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_NF_GENERIC_NAME,
                            target_name,
                            tivxVpacNfGenericProcess,
                            tivxVpacNfGenericCreate,
                            tivxVpacNfGenericDelete,
                            NULL,
                            NULL);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacNfGeneric(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_nf_generic_target_kernel[0]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_nf_generic_target_kernel[0] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_nf_generic_target_kernel[1]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_nf_generic_target_kernel[1] = NULL;
        }
    }
    #endif
}

static void tivxVpacNfGenericFreeMem(tivxVpacNfGenericParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }
        if (NULL != prms->dst16)
        {
            tivxMemFree(prms->dst16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dst16 = NULL;
        }

        tivxMemFree(prms, sizeof(tivxVpacNfGenericParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}
