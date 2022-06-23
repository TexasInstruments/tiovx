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
#include "tivx_kernel_dmpac_sde.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_dmpac_sde_priv.h"
#include "vx_kernels_hwa_target.h"
#include "tivx_kernel_dmpac_sde_generic.h"
#include <math.h>

typedef struct
{
    /* Pointers to inputs and output */
    uint16_t *left16;
    uint16_t *right16;
    uint16_t *dst16;
    uint32_t aligned_width;
    uint32_t aligned_height;
    uint32_t buffer_size;

    SDE_HW_MMR mmr;

} tivxDmpacSdeParams;

static tivx_target_kernel vx_dmpac_sde_target_kernel = NULL;

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
static void tivxDmpacSdeFreeMem(tivxDmpacSdeParams *prms);

static vx_status VX_CALLBACK tivxDmpacSdeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *left_desc;
    tivx_obj_desc_image_t *right_desc;
    tivx_obj_desc_image_t *output_desc;
    tivx_obj_desc_distribution_t *confidence_histogram_desc;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        uint32_t size;
        tivxDmpacSdeParams *prms = NULL;
        void *left_target_ptr;
        void *right_target_ptr;
        void *configuration_target_ptr;
        void *confidence_histogram_target_ptr = NULL;
        void *output_target_ptr;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
        left_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        right_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxDmpacSdeParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            left_target_ptr = tivxMemShared2TargetPtr(&left_desc->mem_ptr[0]);
            right_target_ptr = tivxMemShared2TargetPtr(&right_desc->mem_ptr[0]);
            configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
            output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);
            if( confidence_histogram_desc != NULL)
            {
                confidence_histogram_target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);
            }

            tivxCheckStatus(&status, tivxMemBufferMap(left_target_ptr,
               left_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(right_target_ptr,
               right_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
               configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr,
               output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            if( confidence_histogram_desc != NULL)
            {
                tivxCheckStatus(&status, tivxMemBufferMap(confidence_histogram_target_ptr,
                   confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }

            /* C-model supports only 12-bit in uint16_t container
             * So we may need to translate.  In HW, NF_LSE does this
             */
            lse_reformat_in(left_desc, left_target_ptr, prms->left16, 0, 0);
            lse_reformat_in(right_desc, right_target_ptr, prms->right16, 0, 0);

#ifdef VLAB_HWA

            status = vlab_hwa_process(DMPAC_SDE_BASE_ADDRESS, "DMPAC_SDE", sizeof(SDE_HW_MMR), &prms->mmr);

#else
            status = sde_hw(&prms->mmr);

            if (0 != status)
            {
                status = (vx_status)VX_FAILURE;
            }
#endif
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            lse_reformat_out(left_desc, output_desc, output_target_ptr, prms->dst16, 12, 0);

            if( confidence_histogram_desc != NULL)
            {
                memcpy(confidence_histogram_target_ptr, prms->mmr.hist_bin, 128*sizeof(int32_t));
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(left_target_ptr,
               left_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(right_target_ptr,
               right_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
               configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr,
               output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            if( confidence_histogram_desc != NULL)
            {
                tivxCheckStatus(&status, tivxMemBufferUnmap(confidence_histogram_target_ptr,
                   confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_WRITE_ONLY));
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *left_desc;
        tivxDmpacSdeParams *prms = NULL;

        left_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];

        prms = tivxMemAlloc(sizeof(tivxDmpacSdeParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            uint32_t aligned_width = left_desc->imagepatch_addr[0].dim_x;
            uint32_t aligned_height = left_desc->imagepatch_addr[0].dim_y;

            memset(prms, 0, sizeof(tivxDmpacSdeParams));

            /* Internal buffer should be aligned as per SDE specification */
            if (aligned_width < 128) {
                aligned_width = 128;    /* Minimum width = 128 */
            }
            if (aligned_width & 15) {
                aligned_width += 16;
                aligned_width &= ~15;   /* Must be multiple of 16 */
            }

            if (aligned_height < 64) {
                aligned_height = 64;    /* Minimum height = 64 */
            }
            if (aligned_height & 15) {
                aligned_height += 16;
                aligned_height &= ~15;   /* Must be multiple of 16 */
            }
            if (2048 < aligned_width)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Aligned width should be no greater than 2048\n");
            }
            if (1024 < aligned_height)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                  VX_PRINT(VX_ZONE_ERROR, "Parameter height should be no greater than 1024\n");
            }

            prms->aligned_width = aligned_width;
            prms->aligned_height = aligned_height;
            prms->buffer_size = (aligned_width * aligned_height) * 2;

            prms->left16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->left16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->right16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->right16)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
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
                tivx_dmpac_sde_params_t *params;
                uint32_t disp_max;
                void *params_array_target_ptr;

                tivx_obj_desc_user_data_object_t *params_array;

                params_array = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];

                params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                params = (tivx_dmpac_sde_params_t *)params_array_target_ptr;

                /* When migrating to silicon, the hardware takes different values from what the
                 * CModel took for the following registers:
                 *      IMGRES_REG__iw, IMGRES_REG__ih, SRCHRNG_REG__cfgmax, SRCHRNG_REG__cfgmin
                 */
                prms->mmr.mmr.IMGRES_REG__iw = prms->aligned_width;
                prms->mmr.mmr.IMGRES_REG__ih = prms->aligned_height;

                prms->mmr.mmr.SRCHRNG_REG__cfgmin = (params->disparity_min == 1) ? -3 : 0;
                if( 0 == params->disparity_max ) {
                    disp_max = 63+prms->mmr.mmr.SRCHRNG_REG__cfgmin;
                }
                else if ( 1 == params->disparity_max ) {
                    disp_max = 127+prms->mmr.mmr.SRCHRNG_REG__cfgmin;
                }
                else {
                    disp_max = 191+prms->mmr.mmr.SRCHRNG_REG__cfgmin;
                }
                prms->mmr.mmr.SRCHRNG_REG__cfgmax = disp_max;

                prms->mmr.mmr.CTRL_REG__medfen = params->median_filter_enable;
                prms->mmr.mmr.LRCHCK_REG__diffthld = params->threshold_left_right;
                prms->mmr.mmr.TXTFLT_REG__diffthld = params->threshold_texture;
                prms->mmr.mmr.TXTFLT_REG__txtflten = params->texture_filter_enable;
                prms->mmr.mmr.PNLTY_REG__p1 = params->aggregation_penalty_p1;
                prms->mmr.mmr.PNLTY_REG__p2 = params->aggregation_penalty_p2;
                prms->mmr.mmr.CONFMAPG0_REG__confmap_0 = params->confidence_score_map[0];
                prms->mmr.mmr.CONFMAPG0_REG__confmap_1 = params->confidence_score_map[1];
                prms->mmr.mmr.CONFMAPG1_REG__confmap_2 = params->confidence_score_map[2];
                prms->mmr.mmr.CONFMAPG1_REG__confmap_3 = params->confidence_score_map[3];
                prms->mmr.mmr.CONFMAPG2_REG__confmap_4 = params->confidence_score_map[4];
                prms->mmr.mmr.CONFMAPG2_REG__confmap_5 = params->confidence_score_map[5];
                prms->mmr.mmr.CONFMAPG3_REG__confmap_6 = params->confidence_score_map[6];
                prms->mmr.mmr.CONFMAPG3_REG__confmap_7 = params->confidence_score_map[7];
                prms->mmr.leftImg = prms->left16;
                prms->mmr.rightImg = prms->right16;
                prms->mmr.outImg = prms->dst16;

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
                sizeof(tivxDmpacSdeParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxDmpacSdeFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxDmpacSdeParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxDmpacSdeParams) == size))
        {
            tivxDmpacSdeFreeMem(prms);
        }
    }

    return status;
}

void tivxAddTargetKernelDmpacSde(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_1)
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_SDE, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_sde_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DMPAC_SDE_NAME,
                            target_name,
                            tivxDmpacSdeProcess,
                            tivxDmpacSdeCreate,
                            tivxDmpacSdeDelete,
                            NULL,
                            NULL);
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
}

static void tivxDmpacSdeFreeMem(tivxDmpacSdeParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->left16)
        {
            tivxMemFree(prms->left16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->left16 = NULL;
        }
        if (NULL != prms->right16)
        {
            tivxMemFree(prms->right16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->right16 = NULL;
        }
        if (NULL != prms->dst16)
        {
            tivxMemFree(prms->dst16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dst16 = NULL;
        }
        tivxMemFree(prms, sizeof(tivxDmpacSdeParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}
