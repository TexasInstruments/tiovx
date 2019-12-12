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
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_scale.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "scaler_core.h"
#include "tivx_kernel_vpac_msc.h"
#include "tivx_hwa_vpac_msc_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint16_t *src16;
    uint16_t *dst16[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];
    uint32_t buffer_size_in;
    uint32_t buffer_size_out[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];

    /* To support NV12 format */
    uint16_t *src16_cbcr;
    uint16_t *dst16_cbcr[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];
    uint32_t buffer_size_in_cbcr;
    uint32_t buffer_size_out_cbcr[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];

    msc_config config;

    uint32_t crop_width[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];
    uint32_t crop_height[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT];

} tivxMscScaleParams;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */


static vx_status VX_CALLBACK tivxKernelMscScaleProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelMscScaleCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelMscScaleDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static void tivxVpacMscScaleFreeMem(tivxMscScaleParams *prms);
static void tivxVpacMscScaleInitParams(Scaler_Config *settings);

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);
static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_msc_multi_scale_target_kernel = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscMultiScale(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_0) || (self_cpu == (vx_enum)TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_msc_multi_scale_target_kernel = tivxAddTargetKernelByName(
            TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
            target_name,
            tivxKernelMscScaleProcess,
            tivxKernelMscScaleCreate,
            tivxKernelMscScaleDelete,
            tivxKernelMscScaleControl,
            NULL);
    }
}


void tivxRemoveTargetKernelVpacMscMultiScale(void)
{
    tivxRemoveTargetKernel(vx_vpac_msc_multi_scale_target_kernel);
    vx_vpac_msc_multi_scale_target_kernel = NULL;
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxKernelMscScaleCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    uint32_t cnt;
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *imgIn, *imgOut;
    tivxMscScaleParams *prms = NULL;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxKernelMscScaleCreate: Invalid Parameters\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        imgIn = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX];
        imgOut = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxMscScaleParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL == prms)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxKernelMscScaleCreate: Params allocation error\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        else
        {
            memset(prms, 0x0, sizeof(tivxMscScaleParams));
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            prms->buffer_size_in = imgIn->imagepatch_addr[0].dim_x *
                                   imgIn->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxKernelMscScaleCreate: Input Buffer Alloc Error\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if(((vx_status)VX_SUCCESS == status) && (imgIn->format == (vx_df_image)VX_DF_IMAGE_NV12))
            {
                prms->buffer_size_in_cbcr = imgIn->imagepatch_addr[1].dim_x *
                                            (imgIn->imagepatch_addr[1].dim_y / imgIn->imagepatch_addr[1].step_y) * 2;

                prms->src16_cbcr = tivxMemAlloc(prms->buffer_size_in_cbcr, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->src16_cbcr)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxKernelMscScaleCreate: Input Buffer Alloc Error\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {
                imgOut = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];

                if (NULL != imgOut)
                {
                    prms->buffer_size_out[cnt] =
                        imgOut->imagepatch_addr[0].dim_x *
                        imgOut->imagepatch_addr[0].dim_y * 2;

                    prms->dst16[cnt] = tivxMemAlloc(prms->buffer_size_out[cnt],
                        (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->dst16[cnt])
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                            "tivxKernelMscScaleCreate: Output%d Buffer Alloc Error\n",
                            cnt);
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }

                    if(((vx_status)VX_SUCCESS == status) && (imgIn->format == (vx_df_image)VX_DF_IMAGE_NV12))
                    {
                        prms->buffer_size_out_cbcr[cnt] =
                            imgOut->imagepatch_addr[1].dim_x *
                            (imgOut->imagepatch_addr[1].dim_y / imgOut->imagepatch_addr[1].step_y) * 2;

                        prms->dst16_cbcr[cnt] = tivxMemAlloc(prms->buffer_size_out_cbcr[cnt],
                            (vx_enum)TIVX_MEM_EXTERNAL);
                        if (NULL == prms->dst16_cbcr[cnt])
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                "tivxKernelMscScaleCreate: Output%d Buffer Alloc Error\n",
                                cnt);
                            status = (vx_status)VX_ERROR_NO_MEMORY;
                        }
                    }
                }
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxVpacMscScaleInitParams(&prms->config.settings);

            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxMscScaleParams));
        }
        else
        {
            tivxVpacMscScaleFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelMscScaleProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    uint32_t cnt;
    vx_status status = (vx_status)VX_SUCCESS;
    tivxMscScaleParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_image_t *dst[TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT] = {NULL};
    uint32_t size;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleProcess: Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX];

        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            dst[cnt] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];
        }

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS == status)
        {
            if ((NULL == prms) || (size != sizeof(tivxMscScaleParams)))
            {
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        /* C-model supports only 12-bit in uint16_t container
         * So we may need to translate.  In HW, VPAC_LSE does this
         */
        lse_reformat_in(src, src_target_ptr, prms->src16, 0, 0);

        tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        if(src->format == (vx_df_image)VX_DF_IMAGE_NV12)
        {
            src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[1]);
            tivxMemBufferMap(src_target_ptr, src->mem_size[1],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            /* C-model supports only 12-bit in uint16_t container
             * So we may need to translate.  In HW, VPAC_LSE does this
             */
            lse_reformat_in(src, src_target_ptr, prms->src16_cbcr, 1, 0);

            tivxMemBufferUnmap(src_target_ptr, src->mem_size[1],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t iw = src->imagepatch_addr[0].dim_x;
        uint32_t ih = src->imagepatch_addr[0].dim_y;

        prms->config.settings.G_inWidth[0] = src->imagepatch_addr[0].dim_x;
        prms->config.settings.G_inHeight[0] = src->imagepatch_addr[0].dim_y;

        /* Is it enough to set for just 1 pipe in host-emulation mode? */
        prms->config.settings.unitParams[0].uvMode = 0;

        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != dst[cnt])
            {
                uint32_t ow = dst[cnt]->imagepatch_addr[0].dim_x;
                uint32_t oh = dst[cnt]->imagepatch_addr[0].dim_y;
                uint32_t hzScale = ((float)(4096*iw)/(float)ow) + 0.5f;
                uint32_t vtScale = ((float)(4096*ih)/(float)oh) + 0.5f;

                prms->config.settings.unitParams[cnt].outWidth =
                    dst[cnt]->imagepatch_addr[0].dim_x;
                prms->config.settings.unitParams[cnt].outHeight =
                    dst[cnt]->imagepatch_addr[0].dim_y;
                prms->config.settings.unitParams[cnt].hzScale = hzScale;
                prms->config.settings.unitParams[cnt].vtScale = vtScale;

                /* Control Command provides an interface for setting
                 * init phase information, but currently overriding it
                 * using this equation. */
                prms->config.settings.unitParams[cnt].initPhaseX =
                    (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
                prms->config.settings.unitParams[cnt].initPhaseY =
                    (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
            }
            else
            {
                break;
            }
        }

#ifdef VLAB_HWA

        prms->config.magic = 0xC0DEFACE;
        prms->config.buffer[0]  = prms->src16;
        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != dst[cnt])
            {
                prms->config.buffer[4 + (cnt * 2)]  = prms->dst16[cnt];
            }
            else
            {
                break;
            }
        }

        status = vlab_hwa_process(VPAC_MSC_BASE_ADDRESS, "VPAC_MSC_SCALE", sizeof(msc_config), &prms->config);

#else
        {
            unsigned short *imgInput[2];
            unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

            imgInput[0] = prms->src16;

            for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {
                if (NULL != dst[cnt])
                {
                    imgOutput[cnt] = prms->dst16[cnt];
                }
                else
                {
                    break;
                }
            }

            scaler_top_processing(imgInput, imgOutput, &prms->config.settings);
        }
#endif
    }

    if (((vx_status)VX_SUCCESS == status) && (src->format == (vx_df_image)VX_DF_IMAGE_NV12))
    {
        uint32_t iw = src->imagepatch_addr[1].dim_x;
        uint32_t ih = src->imagepatch_addr[1].dim_y / src->imagepatch_addr[1].step_y;

        prms->config.settings.G_inWidth[0] = src->imagepatch_addr[1].dim_x;
        prms->config.settings.G_inHeight[0] = src->imagepatch_addr[1].dim_y / src->imagepatch_addr[1].step_y;

        /* Is it enough to set for just 1 pipe in host-emulation mode? */
        prms->config.settings.unitParams[0].uvMode = 1;

        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != dst[cnt])
            {
                uint32_t ow = dst[cnt]->imagepatch_addr[1].dim_x;
                uint32_t oh = dst[cnt]->imagepatch_addr[1].dim_y / dst[cnt]->imagepatch_addr[1].step_y;
                uint32_t hzScale = ((float)(4096*iw)/(float)ow) + 0.5f;
                uint32_t vtScale = ((float)(4096*ih)/(float)oh) + 0.5f;

                prms->config.settings.unitParams[cnt].outWidth =
                    dst[cnt]->imagepatch_addr[1].dim_x;
                prms->config.settings.unitParams[cnt].outHeight =
                    dst[cnt]->imagepatch_addr[1].dim_y / dst[cnt]->imagepatch_addr[1].step_y;
                prms->config.settings.unitParams[cnt].hzScale = hzScale;
                prms->config.settings.unitParams[cnt].vtScale = vtScale;

                /* Control Command provides an interface for setting
                 * init phase information, but currently overriding it
                 * using this equation. */
                prms->config.settings.unitParams[cnt].initPhaseX =
                    (((((float)iw/(float)ow) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
                prms->config.settings.unitParams[cnt].initPhaseY =
                    (((((float)ih/(float)oh) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
            }
            else
            {
                break;
            }
        }

#ifdef VLAB_HWA

        prms->config.magic = 0xC0DEFACE;
        prms->config.buffer[0]  = prms->src16_cbcr;
        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != dst[cnt])
            {
                prms->config.buffer[4 + (cnt * 2)]  = prms->dst16_cbcr[cnt];
            }
            else
            {
                break;
            }
        }

        status = vlab_hwa_process(VPAC_MSC_BASE_ADDRESS, "VPAC_MSC_SCALE", sizeof(msc_config), &prms->config);

#else
        {
            unsigned short *imgInput[2];
            unsigned short *imgOutput[SCALER_NUM_PIPES] = {0};

            imgInput[0] = prms->src16_cbcr;

            for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {
                if (NULL != dst[cnt])
                {
                    imgOutput[cnt] = prms->dst16_cbcr[cnt];
                }
                else
                {
                    break;
                }
            }

            scaler_top_processing(imgInput, imgOutput, &prms->config.settings);
        }
#endif
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t stub;
        void *dst_target_ptr;

        stub.valid_roi.start_x = 0;
        stub.valid_roi.start_y = 0;

        for (cnt = 0; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if (NULL != dst[cnt])
            {
                /* Reformat output */
                dst_target_ptr = tivxMemShared2TargetPtr(&dst[cnt]->mem_ptr[0]);
                tivxMemBufferMap(dst_target_ptr, dst[cnt]->mem_size[0],
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

                lse_reformat_out(&stub, dst[cnt], dst_target_ptr,
                    prms->dst16[cnt], 12, 0);

                tivxMemBufferUnmap(dst_target_ptr, dst[cnt]->mem_size[0],
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

                if(dst[cnt]->format == (vx_df_image)VX_DF_IMAGE_NV12)
                {
                    dst_target_ptr = tivxMemShared2TargetPtr(&dst[cnt]->mem_ptr[1]);
                    tivxMemBufferMap(dst_target_ptr, dst[cnt]->mem_size[1],
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);

                    lse_reformat_out(&stub, dst[cnt], dst_target_ptr,
                        prms->dst16_cbcr[cnt], 12, 1);

                    tivxMemBufferUnmap(dst_target_ptr, dst[cnt]->mem_size[1],
                        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY);
                }
            }
            else
            {
                break;
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelMscScaleDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxKernelMscScaleDelete: Invalid Parameters\n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxMscScaleParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxMscScaleParams) == size))
        {
            tivxVpacMscScaleFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    uint32_t                      size;
    tivxMscScaleParams           *prms = NULL;

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxMscScaleParams) == size))
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
                status = tivxVpacMscScaleSetCoeffsCmd(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetInputParamsCmd(prms,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetOutputParamsCmd(prms,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS:
            {
                status = tivxVpacMscScaleSetCropParamsCmd(prms,
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

static void tivxVpacMscScaleFreeMem(tivxMscScaleParams *prms)
{
    uint32_t cnt;

    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size_in, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }
        if (NULL != prms->src16_cbcr)
        {
            tivxMemFree(prms->src16_cbcr, prms->buffer_size_in_cbcr, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16_cbcr = NULL;
        }

        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
        {
            if ((NULL != prms->dst16[cnt]) &&
                (0U != prms->buffer_size_out[cnt]))
            {
                tivxMemFree(prms->dst16[cnt], prms->buffer_size_out[cnt],
                    (vx_enum)TIVX_MEM_EXTERNAL);
                prms->dst16[cnt] = NULL;
            }
            if ((NULL != prms->dst16_cbcr[cnt]) &&
                (0U != prms->buffer_size_out_cbcr[cnt]))
            {
                tivxMemFree(prms->dst16_cbcr[cnt], prms->buffer_size_out_cbcr[cnt],
                    (vx_enum)TIVX_MEM_EXTERNAL);
                prms->dst16_cbcr[cnt] = NULL;
            }
        }

        tivxMemFree(prms, sizeof(tivxMscScaleParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static void tivxVpacMscScaleInitParams(Scaler_Config *settings)
{
    uint32_t i;
    uint32_t weight;
    /* The precision (64 phases) of the bilinear interpolation on the random
     * conformance tests for ORB, 3_1, and DOWN_NEAR is not enough
     * to pass these conformance tests.  It is expected that real images
     * would have better results */

    /* Coefficients for Bilinear Interpolation */
    for(i=0; i<32; i++)
    {
        weight = i<<2;
        settings->coef_mp[0].matrix[i][0] = 0;
        settings->coef_mp[0].matrix[i][1] = 0;
        settings->coef_mp[0].matrix[i][2] = 256-weight;
        settings->coef_mp[0].matrix[i][3] = weight;
        settings->coef_mp[0].matrix[i][4] = 0;
    }
    for(i=0; i<32; i++)
    {
        weight = (i+32)<<2;
        settings->coef_mp[1].matrix[i][0] = 0;
        settings->coef_mp[1].matrix[i][1] = 0;
        settings->coef_mp[1].matrix[i][2] = 256-weight;
        settings->coef_mp[1].matrix[i][3] = weight;
        settings->coef_mp[1].matrix[i][4] = 0;
    }
    /* Coefficients for Nearest Neighbor */
    for(i=0; i<32; i++)
    {
        settings->coef_mp[2].matrix[i][0] = 0;
        settings->coef_mp[2].matrix[i][1] = 0;
        settings->coef_mp[2].matrix[i][2] = 256;
        settings->coef_mp[2].matrix[i][3] = 0;
        settings->coef_mp[2].matrix[i][4] = 0;
    }
    for(i=0; i<32; i++)
    {
        settings->coef_mp[3].matrix[i][0] = 0;
        settings->coef_mp[3].matrix[i][1] = 0;
        settings->coef_mp[3].matrix[i][2] = 0;
        settings->coef_mp[3].matrix[i][3] = 256;
        settings->coef_mp[3].matrix[i][4] = 0;
    }

    /* Be default, 5-tap filter */
    settings->cfg_Kernel[0].Sz_height = 5;
    settings->cfg_Kernel[0].Tpad_sz = 2;
    settings->cfg_Kernel[0].Bpad_sz = 2;

    /* Initializing all scaler outputs to defaults */
    for (i = 0u; i < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; i ++)
    {
        settings->unitParams[i].threadMap = 0;
        settings->unitParams[i].coefShift = 8;

        settings->unitParams[i].filter_mode = 1;

        settings->unitParams[i].phase_mode = 0;
        settings->unitParams[i].hs_coef_sel = 0;
        settings->unitParams[i].vs_coef_sel = 0;

        settings->unitParams[i].x_offset = 0;
        settings->unitParams[i].y_offset = 0;
    }
}

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    void                         *target_ptr;
    tivx_vpac_msc_coefficients_t *coeffs;
    uint32_t                      i, j, idx;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

        if (sizeof(tivx_vpac_msc_coefficients_t) ==
                usr_data_obj->mem_size)
        {
            coeffs = (tivx_vpac_msc_coefficients_t *)target_ptr;
            for (i = 0; i < 4; i ++)
            {
                idx = 0;
                for (j = 0; j < 32; j ++)
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

            for (i = 0; i < 2; i ++)
            {
                for (j = 0; j < 5; j ++)
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

static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

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
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscScaleSetInputParamsCmd: Invalid Kernel Size\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
            }

        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "tivxVpacMscScaleSetInputParamsCmd: Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscScaleSetInputParamsCmd: User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return (status);
}

static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;

    for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);

            tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            if (sizeof(tivx_vpac_msc_output_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;

                prms->config.settings.unitParams[cnt].signedData = out_prms->signed_data;
                prms->config.settings.unitParams[cnt].filter_mode = out_prms->filter_mode;
                prms->config.settings.unitParams[cnt].coefShift = out_prms->coef_shift;
                prms->config.settings.unitParams[cnt].satMode = out_prms->saturation_mode;
                prms->config.settings.unitParams[cnt].x_offset = out_prms->offset_x;
                prms->config.settings.unitParams[cnt].y_offset = out_prms->offset_y;
                prms->config.settings.unitParams[cnt].sp_hs_coef_src = out_prms->single_phase.horz_coef_src;
                prms->config.settings.unitParams[cnt].sp_hs_coef_sel = out_prms->single_phase.horz_coef_sel;
                prms->config.settings.unitParams[cnt].sp_vs_coef_src = out_prms->single_phase.vert_coef_src;
                prms->config.settings.unitParams[cnt].sp_vs_coef_src = out_prms->single_phase.vert_coef_src;

                prms->config.settings.unitParams[cnt].phase_mode = out_prms->multi_phase.phase_mode;
                prms->config.settings.unitParams[cnt].hs_coef_sel = out_prms->multi_phase.horz_coef_sel;
                prms->config.settings.unitParams[cnt].vs_coef_sel = out_prms->multi_phase.vert_coef_sel;
                prms->config.settings.unitParams[cnt].initPhaseX = out_prms->multi_phase.init_phase_x;
                prms->config.settings.unitParams[cnt].initPhaseY = out_prms->multi_phase.init_phase_y;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "Invalid Mem Size for tivx_vpac_msc_output_params_t\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    return (status);
}

static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxMscScaleParams *prms,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt;
    tivx_vpac_msc_crop_params_t      *out_prms = NULL;
    void                             *target_ptr;

    for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
    {
        if (NULL != usr_data_obj[cnt])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[cnt]->mem_ptr);

            tivxMemBufferMap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);

            if (sizeof(tivx_vpac_msc_crop_params_t) ==
                    usr_data_obj[cnt]->mem_size)
            {
                out_prms = (tivx_vpac_msc_crop_params_t *)target_ptr;

                prms->config.settings.unitParams[cnt].x_offset = out_prms->crop_start_x;
                prms->config.settings.unitParams[cnt].y_offset = out_prms->crop_start_y;
                prms->crop_width[cnt] = out_prms->crop_width;
                prms->crop_height[cnt] = out_prms->crop_height;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscScaleSetCropParamsCmd: Invalid Mem Size for Crop Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxMemBufferUnmap(target_ptr, usr_data_obj[cnt]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    return (status);
}
