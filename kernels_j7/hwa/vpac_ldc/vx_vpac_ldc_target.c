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
#include "TI/j7.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "ldc.h"

typedef struct
{
    /* Pointers to inputs and output */
    uint16_t *inY_16;
    uint16_t *inC_16;
    uint16_t *outY0_16;
    uint16_t *outC1_16;
    uint16_t *outY2_16;
    uint16_t *outC3_16;
    uint32_t *mesh;
    uint32_t inY_buffer_size;
    uint32_t inC_buffer_size;
    uint32_t outY0_buffer_size;
    uint32_t outC1_buffer_size;
    uint32_t outY2_buffer_size;
    uint32_t outC3_buffer_size;
    uint32_t mesh_buffer_size;

    ldc_config config;

} tivxVpacLdcParams;

static tivx_target_kernel vx_vpac_ldc_target_kernel = NULL;

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacLdcFreeMem(tivxVpacLdcParams *prms);

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
#if 0
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_user_data_object_t *region_params_desc;
    tivx_obj_desc_image_t *mesh_table_desc;
    tivx_obj_desc_matrix_t *warp_matrix_desc;
    tivx_obj_desc_lut_t *out_2_luma_lut_desc;
    tivx_obj_desc_lut_t *out_3_chroma_lut_desc;
    tivx_obj_desc_user_data_object_t *bandwidth_params_desc;
#endif
    tivx_obj_desc_image_t *in_luma_or_422_desc;
    tivx_obj_desc_image_t *in_chroma_desc;
    tivx_obj_desc_image_t *out_0_luma_or_422_desc;
    tivx_obj_desc_image_t *out_1_chroma_desc;
    tivx_obj_desc_image_t *out_2_luma_or_422_desc;
    tivx_obj_desc_image_t *out_3_chroma_desc;
    tivx_obj_desc_scalar_t *error_status_desc;

    if ( (num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        uint32_t size;
        tivxVpacLdcParams *prms = NULL;
        vx_uint32 error_status_value;

        void *in_luma_or_422_target_ptr = NULL;
        void *in_chroma_target_ptr = NULL;
        void *out_0_luma_or_422_target_ptr = NULL;
        void *out_1_chroma_target_ptr = NULL;
        void *out_2_luma_or_422_target_ptr = NULL;
        void *out_3_chroma_target_ptr = NULL;

#if 0
        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
        region_params_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX];
        mesh_table_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_TABLE_IDX];
        warp_matrix_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
        out_2_luma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_LUT_IDX];
        out_3_chroma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_LUT_IDX];
        bandwidth_params_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_BANDWIDTH_PARAMS_IDX];
#endif
        in_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX];
        in_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX];
        out_0_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX];
        out_1_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX];
        out_2_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_OR_422_IDX];
        out_3_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_IDX];
        error_status_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_ERROR_STATUS_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacLdcParams) == size))
        {
            if( in_luma_or_422_desc != NULL)
            {
                in_luma_or_422_target_ptr = tivxMemShared2TargetPtr(
                    in_luma_or_422_desc->mem_ptr[0].shared_ptr, in_luma_or_422_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(in_luma_or_422_target_ptr,
                    in_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_READ_ONLY);

                lse_reformat_in(in_luma_or_422_desc, in_luma_or_422_target_ptr, prms->inY_16);

                tivxMemBufferUnmap(in_luma_or_422_target_ptr,
                    in_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_READ_ONLY);
            }
            if( in_chroma_desc != NULL)
            {
                in_chroma_target_ptr = tivxMemShared2TargetPtr(
                    in_chroma_desc->mem_ptr[0].shared_ptr, in_chroma_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(in_chroma_target_ptr,
                    in_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_READ_ONLY);

                lse_reformat_in(in_chroma_desc, in_chroma_target_ptr, prms->inC_16);

                tivxMemBufferUnmap(in_chroma_target_ptr,
                    in_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_READ_ONLY);
            }

#ifdef VLAB_HWA

            prms->config.magic = 0xC0DEFACE;
            prms->config.buffer[0]  = prms->inY_16;
            prms->config.buffer[2]  = prms->inC_16;
            prms->config.buffer[4]  = prms->outY0_16;
            prms->config.buffer[6]  = prms->outC1_16;
            prms->config.buffer[8]  = prms->outY2_16;
            prms->config.buffer[10] = prms->outC3_16;

            vlab_hwa_process(VPAC_LDC_BASE_ADDRESS, "VPAC_LDC", sizeof(ldc_config), &prms->config);

#else

            ldc(&prms->config.settings, prms->inY_16, prms->inC_16,
                         prms->outY0_16, prms->outC1_16, prms->outY2_16, prms->outC3_16);
#endif
        }

        if (VX_SUCCESS == status)
        {
            if( out_0_luma_or_422_desc != NULL)
            {
                out_0_luma_or_422_target_ptr = tivxMemShared2TargetPtr(
                    out_0_luma_or_422_desc->mem_ptr[0].shared_ptr, out_0_luma_or_422_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(out_0_luma_or_422_target_ptr,
                    out_0_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);

                lse_reformat_out(in_luma_or_422_desc, out_0_luma_or_422_desc, out_0_luma_or_422_target_ptr, prms->outY0_16, 12);

                tivxMemBufferUnmap(out_0_luma_or_422_target_ptr,
                    out_0_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);
            }
            if( out_1_chroma_desc != NULL)
            {
                out_1_chroma_target_ptr = tivxMemShared2TargetPtr(
                    out_1_chroma_desc->mem_ptr[0].shared_ptr, out_1_chroma_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(out_1_chroma_target_ptr,
                    out_1_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);

                lse_reformat_out(in_chroma_desc, out_1_chroma_desc, out_1_chroma_target_ptr, prms->outC1_16, 12);

                tivxMemBufferUnmap(out_1_chroma_target_ptr,
                    out_1_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);
            }
            if( out_2_luma_or_422_desc != NULL)
            {
                out_2_luma_or_422_target_ptr = tivxMemShared2TargetPtr(
                    out_2_luma_or_422_desc->mem_ptr[0].shared_ptr, out_2_luma_or_422_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(out_2_luma_or_422_target_ptr,
                    out_2_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);

                lse_reformat_out(in_luma_or_422_desc, out_2_luma_or_422_desc, out_2_luma_or_422_target_ptr, prms->outY2_16, 12);

                tivxMemBufferUnmap(out_2_luma_or_422_target_ptr,
                    out_2_luma_or_422_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);
            }
            if( out_3_chroma_desc != NULL)
            {
                out_3_chroma_target_ptr = tivxMemShared2TargetPtr(
                    out_3_chroma_desc->mem_ptr[0].shared_ptr, out_3_chroma_desc->mem_ptr[0].mem_heap_region);
                tivxMemBufferMap(out_3_chroma_target_ptr,
                    out_3_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);

                lse_reformat_out(in_chroma_desc, out_3_chroma_desc, out_3_chroma_target_ptr, prms->outC3_16, 12);

                tivxMemBufferUnmap(out_3_chroma_target_ptr,
                    out_3_chroma_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                    VX_WRITE_ONLY);
            }
        }
        if(NULL != error_status_desc)
        {
            error_status_value = 0;
            error_status_value |= prms->config.settings.flag_pix_buf_overflow;
            error_status_value |= (prms->config.settings.flag_mesh_buf_overflow << 1);
            error_status_value |= (prms->config.settings.flag_out_of_frame_bound << 2);
            error_status_value |= (prms->config.settings.flag_out_of_block_bound << 3);
            error_status_value |= (prms->config.settings.flag_affine_pwarp_overflow << 4);
            error_status_value |= (prms->config.settings.flag_out_of_mesh_block_bound << 5);
            error_status_desc->data.u32 = error_status_value;
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t *configuration_desc;
        tivx_obj_desc_user_data_object_t *region_params_desc;
        tivx_obj_desc_image_t *mesh_table_desc;
        tivx_obj_desc_matrix_t *warp_matrix_desc;
        tivx_obj_desc_lut_t *out_2_luma_lut_desc;
        tivx_obj_desc_lut_t *out_3_chroma_lut_desc;
        //tivx_obj_desc_user_data_object_t *bandwidth_params_desc;
        tivx_obj_desc_image_t *in_luma_or_422_desc;
        tivx_obj_desc_image_t *in_chroma_desc;
        tivx_obj_desc_image_t *out_0_luma_or_422_desc;
        tivx_obj_desc_image_t *out_1_chroma_desc;
        tivx_obj_desc_image_t *out_2_luma_or_422_desc;
        tivx_obj_desc_image_t *out_3_chroma_desc;
        tivxVpacLdcParams *prms = NULL;

        prms = tivxMemAlloc(sizeof(tivxVpacLdcParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVpacLdcParams));

            configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
            region_params_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX];
            mesh_table_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_TABLE_IDX];
            warp_matrix_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
            out_2_luma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_LUT_IDX];
            out_3_chroma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_LUT_IDX];
            //bandwidth_params_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_BANDWIDTH_PARAMS_IDX];
            in_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX];
            in_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX];
            out_0_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX];
            out_1_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX];
            out_2_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_OR_422_IDX];
            out_3_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_IDX];

            if (NULL != in_luma_or_422_desc)
            {
                prms->inY_buffer_size = in_luma_or_422_desc->imagepatch_addr[0].dim_x *
                                        in_luma_or_422_desc->imagepatch_addr[0].dim_y * 2;
                prms->inY_16 = tivxMemAlloc(prms->inY_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->inY_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != in_chroma_desc))
            {
                prms->inC_buffer_size = in_chroma_desc->imagepatch_addr[0].dim_x *
                                        in_chroma_desc->imagepatch_addr[0].dim_y * 2;
                prms->inC_16 = tivxMemAlloc(prms->inC_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->inC_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != out_0_luma_or_422_desc))
            {
                prms->outY0_buffer_size = out_0_luma_or_422_desc->imagepatch_addr[0].dim_x *
                                          out_0_luma_or_422_desc->imagepatch_addr[0].dim_y * 2;
                prms->outY0_16 = tivxMemAlloc(prms->outY0_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->outY0_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != out_1_chroma_desc))
            {
                prms->outC1_buffer_size = out_1_chroma_desc->imagepatch_addr[0].dim_x *
                                          out_1_chroma_desc->imagepatch_addr[0].dim_y * 2;
                prms->outC1_16 = tivxMemAlloc(prms->outC1_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->outC1_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != out_2_luma_or_422_desc))
            {
                prms->outY2_buffer_size = out_2_luma_or_422_desc->imagepatch_addr[0].dim_x *
                                          out_2_luma_or_422_desc->imagepatch_addr[0].dim_y * 2;
                prms->outY2_16 = tivxMemAlloc(prms->outY2_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->outY2_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != out_3_chroma_desc))
            {
                prms->outC3_buffer_size = out_3_chroma_desc->imagepatch_addr[0].dim_x *
                                          out_3_chroma_desc->imagepatch_addr[0].dim_y * 2;
                prms->outC3_16 = tivxMemAlloc(prms->outC3_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->outC3_16)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != mesh_table_desc))
            {
                prms->mesh_buffer_size = mesh_table_desc->imagepatch_addr[0].dim_x *
                                         mesh_table_desc->imagepatch_addr[0].dim_y * 4;
                prms->mesh = tivxMemAlloc(prms->mesh_buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->mesh)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                tivx_vpac_ldc_params_t *params;
                uint32_t data_mode;
                void *configuration_target_ptr;
                void *region_params_target_ptr;

                configuration_target_ptr = tivxMemShared2TargetPtr(
                    configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_heap_region);

                tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                params = (tivx_vpac_ldc_params_t *)configuration_target_ptr;

                prms->config.settings.pixmem_size  = 42;    // must be 42 before running LDC
                prms->config.settings.pixmem_sizeC = 30;    // must be 30 before running LDC
                prms->config.settings.meshmem_size = 10;    // must be 10 before running LDC

                prms->config.settings.en = 1;                                        // LD enable
                prms->config.settings.ip_dfmt = IP_DFMT_12b_UNPACK;                  // LD input pixel format
                prms->config.settings.ld_yint_typ = params->luma_interpolation_type; // Interpolation method for Y data.  0: Bicubic, 1: Bilinear
                prms->config.settings.ld_initx =  params->init_x;                    // compute window starting y, in pixels
                prms->config.settings.ld_inity =  params->init_y;                    // compute window starting x, in pixels

                /* Configure data mode and input resolution */

                if (NULL != in_luma_or_422_desc)
                {
                    if (VX_DF_IMAGE_UYVY == in_luma_or_422_desc->format)
                    {
                        data_mode = DATA_MODE_422;
                    }
                    else if (NULL != in_chroma_desc)
                    {
                        data_mode = DATA_MODE_420;
                    }
                    else
                    {
                        data_mode = DATA_MODE_Y;
                    }

                    prms->config.settings.iw = in_luma_or_422_desc->imagepatch_addr[0].dim_x; // source (distorted) image width, in pixels
                    prms->config.settings.ih = in_luma_or_422_desc->imagepatch_addr[0].dim_y; // source (distorted) image height, in pixels
                }
                else
                {
                    data_mode = DATA_MODE_UV;

                    prms->config.settings.iw = in_chroma_desc->imagepatch_addr[0].dim_x; // source (distorted) image width, in pixels
                    prms->config.settings.ih = in_chroma_desc->imagepatch_addr[0].dim_y; // source (distorted) image height, in pixels
                }
                prms->config.settings.data_mode = data_mode;      // LD input data mode

                if (NULL != out_0_luma_or_422_desc)
                {
                    prms->config.settings.compute_sizew = out_0_luma_or_422_desc->imagepatch_addr[0].dim_x; // compute window width, in pixels
                    prms->config.settings.compute_sizeh = out_0_luma_or_422_desc->imagepatch_addr[0].dim_y; // compute window height, in pixels

                    if((VX_DF_IMAGE_UYVY == out_0_luma_or_422_desc->format) ||
                        (VX_DF_IMAGE_YUYV == out_0_luma_or_422_desc->format))
                    {
                        prms->config.settings.out_in_420 = 0;      // LD 422 to 420 conversion
                    }
                    else
                    {
                        prms->config.settings.out_in_420 = 1;      // LD 422 to 420 conversion
                    }
                }
                else if(NULL != out_1_chroma_desc)
                {
                    prms->config.settings.compute_sizew = out_1_chroma_desc->imagepatch_addr[0].dim_x; // compute window width, in pixels
                    prms->config.settings.compute_sizeh = out_1_chroma_desc->imagepatch_addr[0].dim_y; // compute window height, in pixels

                    if((VX_DF_IMAGE_UYVY == out_1_chroma_desc->format) ||
                        (VX_DF_IMAGE_YUYV == out_1_chroma_desc->format))
                    {
                        prms->config.settings.out_in_420 = 0;      // LD 422 to 420 conversion
                    }
                    else
                    {
                        prms->config.settings.out_in_420 = 1;      // LD 422 to 420 conversion
                    }
                }
                else
                {
                    // Nothing
                }

                /* Configure mesh table */

                if( NULL != mesh_table_desc )
                {
                    int32_t j;
                    int32_t *meshPtr;
                    void *mesh_table_target_ptr;

                    prms->config.settings.ldmapen = 1;     // LD back mapping enable

                    mesh_table_target_ptr = tivxMemShared2TargetPtr(
                        mesh_table_desc->mem_ptr[0].shared_ptr, mesh_table_desc->mem_ptr[0].mem_heap_region);

                    tivxMemBufferMap(mesh_table_target_ptr, mesh_table_desc->mem_size[0],
                        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                    meshPtr = mesh_table_target_ptr;

                    for(j = 0; j < mesh_table_desc->imagepatch_addr[0].dim_y; j++) {
                        uint32_t input_index = (mesh_table_desc->imagepatch_addr[0].stride_y>>2)*j;
                        uint32_t output_index = mesh_table_desc->imagepatch_addr[0].dim_x*j;
                        memcpy(&prms->mesh[output_index], &meshPtr[input_index],  mesh_table_desc->imagepatch_addr[0].dim_x*4);
                    }

                    // derived (not in cfg file directly)
                    prms->config.settings.mesh_table = (int32_t*)prms->mesh;                       // must fill in correctly before running LDC
                    prms->config.settings.table_width = mesh_table_desc->imagepatch_addr[0].dim_x;  // must fill in correctly before running LDC
                    prms->config.settings.table_height = mesh_table_desc->imagepatch_addr[0].dim_y; // must fill in correctly before running LDC

                    prms->config.settings.table_m = params->mesh.subsample_factor;    // Table horizontal subsampling factor, 2^m
                    prms->config.settings.mesh_frame_w = params->mesh.frame_width;    // mesh frame window height
                    prms->config.settings.mesh_frame_h = params->mesh.frame_height;   // mesh frame window width

                    tivxMemBufferUnmap(mesh_table_target_ptr,
                        mesh_table_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                        VX_READ_ONLY);
                }

                /* Configure warp coefficients */

                if( NULL != warp_matrix_desc )
                {
                    void *warp_matrix_target_ptr;

                    warp_matrix_target_ptr = tivxMemShared2TargetPtr(
                        warp_matrix_desc->mem_ptr.shared_ptr, warp_matrix_desc->mem_ptr.mem_heap_region);

                    tivxMemBufferMap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
                        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                    if (VX_TYPE_INT16 == warp_matrix_desc->data_type) /* Direct pass to HW registers */
                    {
                        int16_t *mat_addr;

                        mat_addr = (int16_t *)((uintptr_t)warp_matrix_target_ptr);

                        if(3 == warp_matrix_desc->columns)
                        {
                            prms->config.settings.affine_a = mat_addr[0];
                            prms->config.settings.affine_b = mat_addr[3];
                            prms->config.settings.affine_c = mat_addr[6];
                            prms->config.settings.affine_d = mat_addr[1];
                            prms->config.settings.affine_e = mat_addr[4];
                            prms->config.settings.affine_f = mat_addr[7];
                            prms->config.settings.affine_g = mat_addr[2];
                            prms->config.settings.affine_h = mat_addr[5];
                            prms->config.settings.pwarpen = 1;         // PWARP enable
                        }
                        else
                        {
                            prms->config.settings.affine_a = mat_addr[0];
                            prms->config.settings.affine_b = mat_addr[2];
                            prms->config.settings.affine_c = mat_addr[4];
                            prms->config.settings.affine_d = mat_addr[1];
                            prms->config.settings.affine_e = mat_addr[3];
                            prms->config.settings.affine_f = mat_addr[5];
                            prms->config.settings.affine_g = 0;
                            prms->config.settings.affine_h = 0;
                            prms->config.settings.pwarpen = 0;         // PWARP enable
                        }
                    }
                    else /* Compute HW registers from floating point warp matrix */
                    {
                        float *mat_addr;

                        mat_addr = (float *)((uintptr_t)warp_matrix_target_ptr);

                        if(3 == warp_matrix_desc->columns)
                        {
                            prms->config.settings.affine_a = (int16_t)((mat_addr[0] / mat_addr[9]) * 4096.0f);
                            prms->config.settings.affine_b = (int16_t)((mat_addr[3] / mat_addr[9]) * 4096.0f);
                            prms->config.settings.affine_c = (int16_t)((mat_addr[6] / mat_addr[9]) * 8.0f);
                            prms->config.settings.affine_d = (int16_t)((mat_addr[1] / mat_addr[9]) * 4096.0f);
                            prms->config.settings.affine_e = (int16_t)((mat_addr[4] / mat_addr[9]) * 4096.0f);
                            prms->config.settings.affine_f = (int16_t)((mat_addr[7] / mat_addr[9]) * 8.0f);
                            prms->config.settings.affine_g = (int16_t)((mat_addr[2] / mat_addr[9]) * 8388608.0f);
                            prms->config.settings.affine_h = (int16_t)((mat_addr[5] / mat_addr[9]) * 8388608.0f);
                            prms->config.settings.pwarpen = 1;         // PWARP enable
                        }
                        else
                        {
                            prms->config.settings.affine_a = (int16_t)(mat_addr[0] * 4096.0f);
                            prms->config.settings.affine_b = (int16_t)(mat_addr[2] * 4096.0f);
                            prms->config.settings.affine_c = (int16_t)(mat_addr[4] * 8.0f);
                            prms->config.settings.affine_d = (int16_t)(mat_addr[1] * 4096.0f);
                            prms->config.settings.affine_e = (int16_t)(mat_addr[3] * 4096.0f);
                            prms->config.settings.affine_f = (int16_t)(mat_addr[5] * 8.0f);
                            prms->config.settings.affine_g = 0;
                            prms->config.settings.affine_h = 0;
                            prms->config.settings.pwarpen = 0;         // PWARP enable
                        }
                    }

                    tivxMemBufferUnmap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
                        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
                }
                else
                {
                    prms->config.settings.affine_a = 4096;
                    prms->config.settings.affine_b = 0;
                    prms->config.settings.affine_c = 0;
                    prms->config.settings.affine_d = 0;
                    prms->config.settings.affine_e = 4096;
                    prms->config.settings.affine_f = 0;
                    prms->config.settings.affine_g = 0;
                    prms->config.settings.affine_h = 0;
                    prms->config.settings.pwarpen  = 0;         // PWARP enable
                }

                /* Configure region params */

                region_params_target_ptr = tivxMemShared2TargetPtr(
                    region_params_desc->mem_ptr.shared_ptr, region_params_desc->mem_ptr.mem_heap_region);

                tivxMemBufferMap(region_params_target_ptr,
                    region_params_desc->mem_size, VX_MEMORY_TYPE_HOST,
                    VX_READ_ONLY);

                if( region_params_desc->mem_size == sizeof(tivx_vpac_ldc_region_params_t) )
                {
                    tivx_vpac_ldc_region_params_t *region_params =
                        (tivx_vpac_ldc_region_params_t*)region_params_target_ptr;

                    prms->config.settings.regmode_en = 0;      // Region mode enable.  0: off, 1: on

                    prms->config.settings.ld_obw = region_params->out_block_width;  // output block height, in pixels, for block processing
                    prms->config.settings.ld_obh = region_params->out_block_height; // output block height, in pixels, for block processing
                    prms->config.settings.ld_pad = region_params->pixel_pad;        // pixel padding to determine input block for block processing
                }
                else
                {
                    uint32_t i;

                    tivx_vpac_ldc_subregion_params_t *region_params =
                        (tivx_vpac_ldc_subregion_params_t*)region_params_target_ptr;

                    prms->config.settings.regmode_en = 1;      // Region mode enable.  0: off, 1: on

                    prms->config.settings.ld_sf_width[0]  = region_params->column_width[0]; // subframe width
                    prms->config.settings.ld_sf_width[1]  = region_params->column_width[1];
                    prms->config.settings.ld_sf_width[2]  = region_params->column_width[2];
                    prms->config.settings.ld_sf_height[0] = region_params->row_height[0];   // subframe height
                    prms->config.settings.ld_sf_height[1] = region_params->row_height[1];
                    prms->config.settings.ld_sf_height[2] = region_params->row_height[2];

                    for(i = 0; i < 9; i++) {
                        prms->config.settings.ld_sf_en[i]  = region_params->enable[i];           // subframe enable
                        prms->config.settings.ld_sf_obw[i] = region_params->out_block_width[i];  // output block width, in pixels, for block processing
                        prms->config.settings.ld_sf_obh[i] = region_params->out_block_height[i]; // output block height, in pixels, for block processing
                        prms->config.settings.ld_sf_pad[i] = region_params->pixel_pad[i];        // pixel padding to determine input block for block processing
                    }
                }

                tivxMemBufferUnmap(region_params_target_ptr, region_params_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                /* Configure out2 and/or out3 luts */

                if(NULL != out_2_luma_lut_desc)
                {
                    uint32_t i;
                    uint16_t *lut_addr;
                    void *out_2_luma_lut_target_ptr;

                    out_2_luma_lut_target_ptr = tivxMemShared2TargetPtr(
                        out_2_luma_lut_desc->mem_ptr.shared_ptr, out_2_luma_lut_desc->mem_ptr.mem_heap_region);

                    tivxMemBufferMap(out_2_luma_lut_target_ptr,
                        out_2_luma_lut_desc->mem_size, VX_MEMORY_TYPE_HOST,
                        VX_READ_ONLY);

                    lut_addr = out_2_luma_lut_target_ptr;

                    prms->config.settings.ylut_en = 1;
                    prms->config.settings.yin_bits = params->out_2_luma.in_bits;
                    prms->config.settings.yout_bits = params->out_2_luma.out_bits;

                    for(i = 0; i < 513; i++) {
                        prms->config.settings.ylut[i]  = lut_addr[i];           // subframe enable
                    }
                    tivxMemBufferUnmap(out_2_luma_lut_target_ptr, out_2_luma_lut_desc->mem_size,
                        VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
                }

                if(NULL != out_3_chroma_lut_desc)
                {
                    uint32_t i;
                    uint16_t *lut_addr;
                    void *out_3_chroma_lut_target_ptr;

                    out_3_chroma_lut_target_ptr = tivxMemShared2TargetPtr(
                        out_3_chroma_lut_desc->mem_ptr.shared_ptr, out_3_chroma_lut_desc->mem_ptr.mem_heap_region);

                    tivxMemBufferMap(out_3_chroma_lut_target_ptr,
                        out_3_chroma_lut_desc->mem_size, VX_MEMORY_TYPE_HOST,
                        VX_READ_ONLY);

                    lut_addr = out_3_chroma_lut_target_ptr;

                    prms->config.settings.clut_en = 1;
                    prms->config.settings.cin_bits = params->out_3_chroma.in_bits;
                    prms->config.settings.cout_bits = params->out_3_chroma.out_bits;

                    for(i = 0; i < 513; i++) {
                        prms->config.settings.clut[i]  = lut_addr[i];           // subframe enable
                    }
                }

                // additional (not in cfg file)
                prms->config.settings.DDR_S = 6;                        // must be 6 before running LDC

                tivxMemBufferUnmap(configuration_target_ptr, configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxVpacLdcParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacLdcFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacLdcParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacLdcParams) == size))
        {
            tivxVpacLdcFreeMem(prms);
        }
    }

    return status;
}

void tivxAddTargetKernelVpacLdc()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_LDC1,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_ldc_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            NULL,
                            NULL);

/* For now, this is one LDC instance, if we add a second, we ened to do something
 * like below here.
 */
#if 0
        strncpy(target_name, TIVX_TARGET_VPAC_LDC2,
            TIVX_TARGET_MAX_NAME);

        vx_vpac_ldc2_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            NULL,
                            NULL);
#endif
    }
}

void tivxRemoveTargetKernelVpacLdc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_ldc_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_vpac_ldc_target_kernel = NULL;
    }
}

static void tivxVpacLdcFreeMem(tivxVpacLdcParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->inY_16)
        {
            tivxMemFree(prms->inY_16, prms->inY_buffer_size, TIVX_MEM_EXTERNAL);
            prms->inY_16 = NULL;
        }
        if (NULL != prms->inC_16)
        {
            tivxMemFree(prms->inC_16, prms->inC_buffer_size, TIVX_MEM_EXTERNAL);
            prms->inC_16 = NULL;
        }
        if (NULL != prms->outY0_16)
        {
            tivxMemFree(prms->outY0_16, prms->outY0_buffer_size, TIVX_MEM_EXTERNAL);
            prms->outY0_16 = NULL;
        }
        if (NULL != prms->outC1_16)
        {
            tivxMemFree(prms->outC1_16, prms->outC1_buffer_size, TIVX_MEM_EXTERNAL);
            prms->outC1_16 = NULL;
        }
        if (NULL != prms->outY2_16)
        {
            tivxMemFree(prms->outY2_16, prms->outY2_buffer_size, TIVX_MEM_EXTERNAL);
            prms->outY2_16 = NULL;
        }
        if (NULL != prms->outC3_16)
        {
            tivxMemFree(prms->outC3_16, prms->outC3_buffer_size, TIVX_MEM_EXTERNAL);
            prms->outC3_16 = NULL;
        }
        if (NULL != prms->mesh)
        {
            tivxMemFree(prms->mesh, prms->mesh_buffer_size, TIVX_MEM_EXTERNAL);
            prms->mesh = NULL;
        }
        tivxMemFree(prms, sizeof(tivxVpacLdcParams), TIVX_MEM_EXTERNAL);
    }
}
