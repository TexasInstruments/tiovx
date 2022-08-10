/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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

#include "vx_vpac_ldc_target_sim_priv.h"

#include "tivx_hwa_vpac_ldc_priv.h"
#include "vx_kernels_hwa_target.h"

/* #define ENABLE_DEBUG_PRINT */

#ifdef ENABLE_DEBUG_PRINT
#include "stdio.h"
#endif

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_vpac_ldc_target_kernel[2] = {NULL};

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
static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacLdcFreeMem(tivxVpacLdcParams *prms);

static void tivxVpacLdcSetMeshParams(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    tivx_obj_desc_image_t *mesh_img_desc, uint32_t *mesh);
static void tivxVpacLdcSetRegionParams(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *region_params_desc);

void tivxVpacLdcSetWarpParams(ldc_settings *settings,
    tivx_obj_desc_matrix_t *warp_matrix_desc);
static vx_status tivxVpacLdcSetLutParamsCmd(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *luma_user_desc,
    tivx_obj_desc_user_data_object_t *chroma_user_desc);

#ifdef ENABLE_DEBUG_PRINT
void print_csettings(ldc_settings *settings)
{
    printf("pixmem_size     %d\n", settings->pixmem_size);
    printf("pixmem_sizeC    %d\n", settings->pixmem_sizeC);
    printf("meshmem_size    %d\n", settings->meshmem_size);

    printf("en              %d\n", settings->en);
    printf("ldmapen         %d\n", settings->ldmapen);
    printf("data_mode       %d\n", settings->data_mode);
    printf("out_in_420      %d\n", settings->out_in_420);
    printf("ip_dfmt         %d\n", settings->ip_dfmt);
    printf("pwarpen         %d\n", settings->pwarpen);
    printf("ld_yint_typ     %d\n", settings->ld_yint_typ);
    printf("regmode_en      %d\n", settings->regmode_en);
    printf("table_m         %d\n", settings->table_m);
    printf("mesh_frame_w    %d\n", settings->mesh_frame_w);
    printf("mesh_frame_h    %d\n", settings->mesh_frame_h);
    printf("compute_sizew   %d\n", settings->compute_sizew);
    printf("compute_sizeh   %d\n", settings->compute_sizeh);
    printf("ld_initx        %d\n", settings->ld_initx);
    printf("ld_inity        %d\n", settings->ld_inity);
    printf("iw              %d\n", settings->iw);
    printf("ih              %d\n", settings->ih);
    printf("ld_obw          %d\n", settings->ld_obw);
    printf("ld_obh          %d\n", settings->ld_obh);
    printf("ld_pad          %d\n", settings->ld_pad);

    printf("affine_a = %d\n", settings->affine_a);
    printf("affine_b = %d\n", settings->affine_b);
    printf("affine_c = %d\n", settings->affine_c);
    printf("affine_d = %d\n", settings->affine_d);
    printf("affine_e = %d\n", settings->affine_e);
    printf("affine_f = %d\n", settings->affine_f);
    printf("affine_g = %d\n", settings->affine_g);
    printf("affine_h = %d\n", settings->affine_h);
}
#endif

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *in_img;
    tivx_obj_desc_image_t *out_img[2];

    if ( (num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        int i, j;
        uint32_t size;
        tivxVpacLdcParams *prms = NULL;
        uint32_t num_planes;
        uint8_t output_bits = 12;

        void *target_ptr = NULL;

        in_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX];
        out_img[0] = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
        out_img[1] = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacLdcParams) == size))
        {
            if(IP_DFMT_8b == prms->config.settings.ip_dfmt)
            {
                output_bits = 8;
            }

            num_planes = 1U;

            if (((vx_df_image)VX_DF_IMAGE_NV12 == in_img->format) ||
                ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == in_img->format))
            {
                num_planes = 2U;
            }

            /* Cmodel doesn't support UYVY on same plane, first separate */
            if ((vx_df_image)VX_DF_IMAGE_UYVY == in_img->format)
            {
                target_ptr = tivxMemShared2TargetPtr(&in_img->mem_ptr[0]);
                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr,
                    in_img->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                    (vx_enum)VX_READ_ONLY));

                lse_deinterleave_422(in_img, target_ptr, prms->inY_16, prms->inC_16, 8);
            }
            else
            {
                for (i = 0; i < num_planes; i ++)
                {
                    target_ptr = tivxMemShared2TargetPtr(&in_img->mem_ptr[i]);
                    tivxCheckStatus(&status, tivxMemBufferMap(target_ptr,
                        in_img->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                        (vx_enum)VX_READ_ONLY));

                    if (i == 0)
                    {
                        lse_reformat_in(in_img, target_ptr, prms->inY_16, 0, 1);
                    }
                    else
                    {
                        lse_reformat_in(in_img, target_ptr, prms->inC_16, 1, 1);
                    }

                    tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr,
                        in_img->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                        (vx_enum)VX_READ_ONLY));
                }
            }

            if ((vx_df_image)VX_DF_IMAGE_U16 == in_img->format)
            {
                if(0 == prms->input_align_12bit)
                {
                    for(i=0; i < (in_img->width * in_img->height); i++)
                    {
                        prms->inY_16[i] = prms->inY_16[i] & 0xFFF;
                    }
                }
                else
                {
                    for(i=0; i < (in_img->width * in_img->height); i++)
                    {
                        prms->inY_16[i] = prms->inY_16[i] >> 4;
                    }
                }
            }

#ifdef ENABLE_DEBUG_PRINT
            print_csettings(&prms->config.settings);
#endif

#ifdef VLAB_HWA

            prms->config.magic = 0xC0DEFACE;
            prms->config.buffer[0]  = prms->inY_16;
            prms->config.buffer[2]  = prms->inC_16;
            prms->config.buffer[4]  = prms->outY_16[0];
            prms->config.buffer[6]  = prms->outC_16[0];
            prms->config.buffer[8]  = prms->outY_16[1];
            prms->config.buffer[10] = prms->outC_16[1];

            vlab_hwa_process(VPAC_LDC_BASE_ADDRESS, "VPAC_LDC", sizeof(ldc_config), &prms->config);

#else

            ldc(&prms->config.settings, prms->inY_16, prms->inC_16,
                         prms->outY_16[0], prms->outC_16[0], prms->outY_16[1], prms->outC_16[1]);
#endif
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            /* For each output */
            for(j = 0; j < 2; j++)
            {
                if (NULL != out_img[j])
                {
                    num_planes = 1U;

                    if (((vx_df_image)VX_DF_IMAGE_NV12 == out_img[j]->format) ||
                        ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == out_img[j]->format))
                    {
                        num_planes = 2U;
                    }

                    /* Cmodel doesn't support UYVY on same plane, first separate */
                    if (((vx_df_image)VX_DF_IMAGE_UYVY == out_img[j]->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == out_img[j]->format))
                    {
                        target_ptr = tivxMemShared2TargetPtr(&out_img[j]->mem_ptr[0]);
                        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr,
                            out_img[j]->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                            (vx_enum)VX_WRITE_ONLY));

                        lse_interleave_422(in_img, out_img[j], target_ptr, prms->outY_16[j], prms->outC_16[j], 8);
                    }
                    else
                    {
                        for (i = 0; i < num_planes; i ++)
                        {
                            target_ptr = tivxMemShared2TargetPtr(&out_img[j]->mem_ptr[i]);
                            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr,
                                out_img[j]->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                                (vx_enum)VX_WRITE_ONLY));

                            if (i == 0)
                            {
                                lse_reformat_out(in_img, out_img[j], target_ptr, prms->outY_16[j], output_bits, 0);
                            }
                            else
                            {
                                lse_reformat_out(in_img, out_img[j], target_ptr, prms->outC_16[j], output_bits, 1);
                            }

                            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr,
                                out_img[j]->mem_size[i], (vx_enum)VX_MEMORY_TYPE_HOST,
                                (vx_enum)VX_WRITE_ONLY));
                        }
                    }
                }
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_matrix_t           *warp_matrix_desc = NULL;
    tivx_obj_desc_user_data_object_t *region_prms_desc = NULL;
    tivx_obj_desc_user_data_object_t *mesh_prms_desc = NULL;
    tivx_obj_desc_image_t            *mesh_img_desc = NULL;
    tivx_obj_desc_image_t            *in_img_desc = NULL;
    tivx_obj_desc_image_t            *out0_img_desc = NULL;
    tivx_obj_desc_image_t            *out1_img_desc = NULL;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;
    tivxVpacLdcParams                *prms = NULL;

    if ( (num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {

        prms = tivxMemAlloc(sizeof(tivxVpacLdcParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVpacLdcParams));

            configuration_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
            warp_matrix_desc = (tivx_obj_desc_matrix_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
            region_prms_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PRMS_IDX];
            mesh_prms_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_PRMS_IDX];
            mesh_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_IMG_IDX];
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX];
            out0_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
            out1_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
            dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_DCC_DB_IDX];

            if (NULL != in_img_desc)
            {
                prms->inY_buffer_size = in_img_desc->imagepatch_addr[0].dim_x *
                                        in_img_desc->imagepatch_addr[0].dim_y * 2;
                prms->inY_16 = tivxMemAlloc(prms->inY_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->inY_16)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }

                if (((vx_status)VX_SUCCESS == status) &&
                    (((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) ||
                     ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == in_img_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format)))
                {
                    prms->inC_buffer_size = in_img_desc->imagepatch_addr[0].dim_x *
                                            in_img_desc->imagepatch_addr[0].dim_y * 2;
                    prms->inC_16 = tivxMemAlloc(prms->inC_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->inC_16)
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != out0_img_desc))
            {
                prms->outY0_buffer_size = out0_img_desc->imagepatch_addr[0].dim_x *
                                          out0_img_desc->imagepatch_addr[0].dim_y * 2;
                prms->outY_16[0] = tivxMemAlloc(prms->outY0_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->outY_16[0])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }

                if (((vx_status)VX_SUCCESS == status) &&
                    (((vx_df_image)VX_DF_IMAGE_NV12 == out0_img_desc->format) ||
                     ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == out0_img_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format)))
                {
                    prms->outC1_buffer_size = out0_img_desc->imagepatch_addr[0].dim_x *
                                              out0_img_desc->imagepatch_addr[0].dim_y * 2;
                    prms->outC_16[0] = tivxMemAlloc(prms->outC1_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->outC_16[0])
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != out1_img_desc))
            {
                prms->outY2_buffer_size = out1_img_desc->imagepatch_addr[0].dim_x *
                                          out1_img_desc->imagepatch_addr[0].dim_y * 2;
                prms->outY_16[1] = tivxMemAlloc(prms->outY2_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->outY_16[1])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }

                if (((vx_status)VX_SUCCESS == status) &&
                    (((vx_df_image)VX_DF_IMAGE_NV12 == out1_img_desc->format) ||
                     ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == out1_img_desc->format) ||
                     ((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format)))
                {
                    prms->outC3_buffer_size = out1_img_desc->imagepatch_addr[0].dim_x *
                                              out1_img_desc->imagepatch_addr[0].dim_y * 2;
                    prms->outC_16[1] = tivxMemAlloc(prms->outC3_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                    if (NULL == prms->outC_16[1])
                    {
                        status = (vx_status)VX_ERROR_NO_MEMORY;
                    }
                }
            }

            if (((vx_status)VX_SUCCESS == status) && (NULL != mesh_img_desc))
            {
                prms->mesh_buffer_size = mesh_img_desc->imagepatch_addr[0].dim_x *
                                         mesh_img_desc->imagepatch_addr[0].dim_y * 4;
                prms->mesh = tivxMemAlloc(prms->mesh_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->mesh)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tivx_vpac_ldc_params_t *params;
                uint32_t ip_dfmt, data_mode;
                void *configuration_target_ptr;

                configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                params = (tivx_vpac_ldc_params_t *)configuration_target_ptr;

                prms->config.settings.pixmem_size  = 42;    // must be 42 before running LDC
                prms->config.settings.pixmem_sizeC = 30;    // must be 30 before running LDC
                prms->config.settings.meshmem_size = 10;    // must be 10 before running LDC

                prms->config.settings.en = 1u;  // LD enable
                prms->config.settings.ld_yint_typ = params->luma_interpolation_type; // Interpolation method for Y data.  0: Bicubic, 1: Bilinear
                prms->config.settings.ld_initx =  params->init_x;                    // compute window starting y, in pixels
                prms->config.settings.ld_inity =  params->init_y;                    // compute window starting x, in pixels


                /* Configure input format */
                if ((vx_df_image)VX_DF_IMAGE_U16 == in_img_desc->format)
                {
                    ip_dfmt = IP_DFMT_12b_UNPACK;
                }
                else if (((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == in_img_desc->format) ||
                         ((vx_df_image)TIVX_DF_IMAGE_P12 == in_img_desc->format))
                {
                    ip_dfmt = IP_DFMT_12b_PACK;
                }
                else
                {
                    ip_dfmt = IP_DFMT_8b;
                }

                prms->config.settings.ip_dfmt = ip_dfmt;                  // LD input pixel format

                /* Configure data mode and input resolution */
                if ((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format)
                {
                    data_mode = DATA_MODE_422;
                }
                else if (((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format) ||
                         ((vx_df_image)TIVX_DF_IMAGE_NV12_P12 == in_img_desc->format))
                {
                    data_mode = DATA_MODE_420;
                }
                else
                {
                    if (TIVX_VPAC_LDC_MODE_LUMA_ONLY == params->yc_mode)
                    {
                        data_mode = DATA_MODE_Y;
                    }
                    else
                    {
                        data_mode = DATA_MODE_UV;
                    }
                }

                prms->config.settings.iw = in_img_desc->imagepatch_addr[0].dim_x; // source (distorted) image width, in pixels
                prms->config.settings.ih = in_img_desc->imagepatch_addr[0].dim_y; // source (distorted) image height, in pixels
                prms->config.settings.data_mode = data_mode;      // LD input data mode
                prms->input_align_12bit = params->input_align_12bit;

                if (NULL != out0_img_desc)
                {
                    prms->config.settings.compute_sizew = out0_img_desc->imagepatch_addr[0].dim_x; // compute window width, in pixels
                    prms->config.settings.compute_sizeh = out0_img_desc->imagepatch_addr[0].dim_y; // compute window height, in pixels

                    if(((vx_df_image)VX_DF_IMAGE_UYVY == out0_img_desc->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == out0_img_desc->format))
                    {
                        prms->config.settings.out_in_420 = 0;      // LD 422 to 420 conversion
                    }
                    else
                    {
                        prms->config.settings.out_in_420 = 1;      // LD 422 to 420 conversion
                    }
                }

                if (NULL != out1_img_desc)
                {
                    prms->config.settings.compute_sizew = out1_img_desc->imagepatch_addr[0].dim_x; // compute window width, in pixels
                    prms->config.settings.compute_sizeh = out1_img_desc->imagepatch_addr[0].dim_y; // compute window height, in pixels

                    if(((vx_df_image)VX_DF_IMAGE_UYVY == out1_img_desc->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == out1_img_desc->format))
                    {
                        prms->config.settings.out_in_420 = 0;      // LD 422 to 420 conversion
                    }
                    else
                    {
                        prms->config.settings.out_in_420 = 1;      // LD 422 to 420 conversion
                    }
                }

                prms->ldcObj.sensor_dcc_id = params->dcc_camera_id;

                tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr, configuration_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }

            if (NULL != dcc_buf_desc)
            {
                status = tivxVpacLdcSetParamsFromDcc(&prms->ldcObj, dcc_buf_desc);
                if(status == VX_SUCCESS)
                {
                    /* In one shot, syncronize the drv data structures to the sim data structures */
                    status = tivxVpacLdcSetConfigInSim(prms);
                    if (status != VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                            "Failed to Parse and Set ldc Params to simulator data structures\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS returned 0x%x\n", status);
                }
            }
            else
            {
                /* Configure mesh table */
                tivxVpacLdcSetMeshParams(&prms->config.settings,
                    mesh_prms_desc, mesh_img_desc, prms->mesh);

                tivxVpacLdcSetRegionParams(&prms->config.settings,
                    region_prms_desc);

                /* Configure warp coefficients */
                tivxVpacLdcSetWarpParams(&prms->config.settings,
                    warp_matrix_desc);
            }

            prms->config.settings.ylut_en = 0;
            prms->config.settings.clut_en = 0;

            /* additional (not in cfg file) */
            prms->config.settings.DDR_S = 6;                        /* must be 6 before running LDC */

        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
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
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
    {
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacLdcParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacLdcParams) == size))
        {
            tivxVpacLdcFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxVpacLdcParams *prms = NULL;

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacLdcParams) == size))
        {
            status = (vx_status)VX_SUCCESS;
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    switch (node_cmd_id)
    {
        case TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS:
            status = tivxVpacLdcSetLutParamsCmd(&prms->config.settings,
                (tivx_obj_desc_user_data_object_t *)obj_desc[0U],
                (tivx_obj_desc_user_data_object_t *)obj_desc[1U]);
            break;
        case TIVX_VPAC_LDC_CMD_SET_LDC_PARAMS:
        {
            if (NULL != obj_desc[TIVX_VPAC_LDC_SET_PARAMS_WARP_MATRIX_IDX])
            {
                tivxVpacLdcSetWarpParams(&prms->config.settings,
                    (tivx_obj_desc_matrix_t *)obj_desc
                        [TIVX_VPAC_LDC_SET_PARAMS_WARP_MATRIX_IDX]);
            }
            break;
        }
        case TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS:
        {
            status = 0;
            tivx_obj_desc_user_data_object_t *dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[0];
            if (NULL != dcc_buf_desc)
            {
                status = tivxVpacLdcSetParamsFromDcc(&prms->ldcObj, dcc_buf_desc);
                if(status == VX_SUCCESS)
                {
                    /* In one shot, syncronize the drv data structures to the sim data structures */
                    status = tivxVpacLdcSetConfigInSim(prms);
                    if (status != VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                            "Failed to Parse and Set ldc Params to simulator data structures\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxVpacLdcSetParamsFromDcc returned 0x%x\n", status);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS : dcc_buf_desc is NULL\n", status);
                status = (vx_status)VX_FAILURE;
            }
            break;
        }
    }
    return status;
}

void tivxAddTargetKernelVpacLdc()
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_MAX_NAME);
        vx_vpac_ldc_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_LDC1, TIVX_TARGET_MAX_NAME);
        vx_vpac_ldc_target_kernel[1] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacLdc()
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_ldc_target_kernel[0]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_ldc_target_kernel[0] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_ldc_target_kernel[1]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_ldc_target_kernel[1] = NULL;
        }
    }
    #endif
}

static void tivxVpacLdcFreeMem(tivxVpacLdcParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->inY_16)
        {
            tivxMemFree(prms->inY_16, prms->inY_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->inY_16 = NULL;
        }
        if (NULL != prms->inC_16)
        {
            tivxMemFree(prms->inC_16, prms->inC_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->inC_16 = NULL;
        }
        if (NULL != prms->outY_16[0])
        {
            tivxMemFree(prms->outY_16[0], prms->outY0_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->outY_16[0] = NULL;
        }
        if (NULL != prms->outC_16[0])
        {
            tivxMemFree(prms->outC_16[0], prms->outC1_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->outC_16[0] = NULL;
        }
        if (NULL != prms->outY_16[1])
        {
            tivxMemFree(prms->outY_16[1], prms->outY2_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->outY_16[1] = NULL;
        }
        if (NULL != prms->outC_16[1])
        {
            tivxMemFree(prms->outC_16[1], prms->outC3_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->outC_16[1] = NULL;
        }
        if (NULL != prms->mesh)
        {
            tivxMemFree(prms->mesh, prms->mesh_buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->mesh = NULL;
        }
        tivxMemFree(prms, sizeof(tivxVpacLdcParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static void tivxVpacLdcSetMeshParams(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    tivx_obj_desc_image_t *mesh_img_desc, uint32_t *mesh)
{
    tivx_vpac_ldc_mesh_params_t *mesh_prms = NULL;
    void                        *target_ptr;
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != mesh_prms_desc) && (NULL != mesh_img_desc))
    {
        target_ptr = tivxMemShared2TargetPtr(&mesh_prms_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, mesh_prms_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_mesh_params_t) == mesh_prms_desc->mem_size)
        {
            int32_t j;
            int32_t *meshPtr;
            void *mesh_table_target_ptr;

            mesh_prms = (tivx_vpac_ldc_mesh_params_t *)target_ptr;

            settings->ldmapen = 1;     // LD back mapping enable

            mesh_table_target_ptr = tivxMemShared2TargetPtr(&mesh_img_desc->mem_ptr[0]);

            tivxCheckStatus(&status, tivxMemBufferMap(mesh_table_target_ptr, mesh_img_desc->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            meshPtr = mesh_table_target_ptr;

            for(j = 0; j < mesh_img_desc->imagepatch_addr[0].dim_y; j++) {
                uint32_t input_index = (mesh_img_desc->imagepatch_addr[0].stride_y>>2)*j;
                uint32_t output_index = mesh_img_desc->imagepatch_addr[0].dim_x*j;
                xyMeshSwapCpy((uint32_t*)&mesh[output_index], (uint32_t*)&meshPtr[input_index],  mesh_img_desc->imagepatch_addr[0].dim_x);
            }

            // derived (not in cfg file directly)
            settings->mesh_table = (int32_t*)mesh;                          // must fill in correctly before running LDC
            settings->table_width = mesh_img_desc->imagepatch_addr[0].dim_x;  // must fill in correctly before running LDC
            settings->table_height = mesh_img_desc->imagepatch_addr[0].dim_y; // must fill in correctly before running LDC

            tivxCheckStatus(&status, tivxMemBufferUnmap(mesh_table_target_ptr,
                mesh_img_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));

            settings->table_m      = mesh_prms->subsample_factor;
            settings->mesh_frame_w = mesh_prms->mesh_frame_width;
            settings->mesh_frame_h = mesh_prms->mesh_frame_height;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
        }
    }
    else
    {
        settings->ldmapen = 0;     // LD back mapping enable
    }
}

static void tivxVpacLdcSetRegionParams(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *region_params_desc)
{
    void                        *target_ptr;
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != region_params_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&region_params_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, region_params_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_region_params_t) == region_params_desc->mem_size)
        {
            tivx_vpac_ldc_region_params_t *region_params =
                (tivx_vpac_ldc_region_params_t*)target_ptr;

            settings->regmode_en = 0;      // Region mode enable.  0: off, 1: on

            settings->ld_obw = region_params->out_block_width;  // output block height, in pixels, for block processing
            settings->ld_obh = region_params->out_block_height; // output block height, in pixels, for block processing
            settings->ld_pad = region_params->pixel_pad;        // pixel padding to determine input block for block processing
        }
        else
        {
            uint32_t i, j, idx;

            tivx_vpac_ldc_multi_region_params_t *region_params =
                (tivx_vpac_ldc_multi_region_params_t*)target_ptr;

            settings->regmode_en = 1;      // Region mode enable.  0: off, 1: on

            for (i = 0; i < 3; i ++)
            {
                for (j = 0; j < 3; j ++)
                {
                    idx = (i * 3) + j;
                    settings->ld_sf_en[idx]  = region_params->reg_params[i][j].enable;           // subframe enable
                    settings->ld_sf_obw[idx] = region_params->reg_params[i][j].out_block_width;  // output block width, in pixels, for block processing
                    settings->ld_sf_obh[idx] = region_params->reg_params[i][j].out_block_height; // output block height, in pixels, for block processing
                    settings->ld_sf_pad[idx] = region_params->reg_params[i][j].pixel_pad;        // pixel padding to determine input block for block processing
                }
                settings->ld_sf_width[i]  = region_params->reg_width[i]; // subframe width
                settings->ld_sf_height[i]  = region_params->reg_height[i]; // subframe width
            }
        }
    }
    else
    {
        settings->ldmapen = 0u;
        settings->ld_obw = TIVX_VPAC_LDC_DEF_BLOCK_WIDTH;
        settings->ld_obh = TIVX_VPAC_LDC_DEF_BLOCK_HEIGHT;
        settings->ld_pad = TIVX_VPAC_LDC_DEF_PIXEL_PAD;
    }
}

void tivxVpacLdcSetWarpParams(ldc_settings *settings,
    tivx_obj_desc_matrix_t *warp_matrix_desc)
{
    void *warp_matrix_target_ptr;
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != warp_matrix_desc)
    {
        warp_matrix_target_ptr = tivxMemShared2TargetPtr(&warp_matrix_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if ((vx_enum)VX_TYPE_INT16 == warp_matrix_desc->data_type) /* Direct pass to HW registers */
        {
            int16_t *mat_addr;

            mat_addr = (int16_t *)((uintptr_t)warp_matrix_target_ptr);

            if(3 == warp_matrix_desc->columns)
            {
                settings->affine_a = mat_addr[0];
                settings->affine_b = mat_addr[3];
                settings->affine_c = mat_addr[6];
                settings->affine_d = mat_addr[1];
                settings->affine_e = mat_addr[4];
                settings->affine_f = mat_addr[7];
                settings->affine_g = mat_addr[2];
                settings->affine_h = mat_addr[5];
                settings->pwarpen = 1;         // PWARP enable
            }
            else
            {
                settings->affine_a = mat_addr[0];
                settings->affine_b = mat_addr[2];
                settings->affine_c = mat_addr[4];
                settings->affine_d = mat_addr[1];
                settings->affine_e = mat_addr[3];
                settings->affine_f = mat_addr[5];
                settings->affine_g = 0;
                settings->affine_h = 0;
                settings->pwarpen = 0;         // PWARP enable
            }
        }
        else /* Compute HW registers from floating point warp matrix */
        {
            float *mat_addr;

            mat_addr = (float *)((uintptr_t)warp_matrix_target_ptr);

            if(3 == warp_matrix_desc->columns)
            {
                settings->affine_a = (int16_t)((mat_addr[0] / mat_addr[8]) * 4096.0f);
                settings->affine_b = (int16_t)((mat_addr[3] / mat_addr[8]) * 4096.0f);
                settings->affine_c = (int16_t)((mat_addr[6] / mat_addr[8]) * 8.0f);
                settings->affine_d = (int16_t)((mat_addr[1] / mat_addr[8]) * 4096.0f);
                settings->affine_e = (int16_t)((mat_addr[4] / mat_addr[8]) * 4096.0f);
                settings->affine_f = (int16_t)((mat_addr[7] / mat_addr[8]) * 8.0f);
                settings->affine_g = (int16_t)((mat_addr[2] / mat_addr[8]) * 8388608.0f);
                settings->affine_h = (int16_t)((mat_addr[5] / mat_addr[8]) * 8388608.0f);
                settings->pwarpen = 1;         // PWARP enable
            }
            else
            {
                settings->affine_a = (int16_t)(mat_addr[0] * 4096.0f);
                settings->affine_b = (int16_t)(mat_addr[2] * 4096.0f);
                settings->affine_c = (int16_t)(mat_addr[4] * 8.0f);
                settings->affine_d = (int16_t)(mat_addr[1] * 4096.0f);
                settings->affine_e = (int16_t)(mat_addr[3] * 4096.0f);
                settings->affine_f = (int16_t)(mat_addr[5] * 8.0f);
                settings->affine_g = 0;
                settings->affine_h = 0;
                settings->pwarpen = 0;         // PWARP enable
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        settings->affine_a = 4096;
        settings->affine_b = 0;
        settings->affine_c = 0;
        settings->affine_d = 0;
        settings->affine_e = 4096;
        settings->affine_f = 0;
        settings->affine_g = 0;
        settings->affine_h = 0;
        settings->pwarpen  = 0;         // PWARP enable
    }
}

static vx_status tivxVpacLdcSetLutParamsCmd(ldc_settings *settings,
    tivx_obj_desc_user_data_object_t *luma_user_desc,
    tivx_obj_desc_user_data_object_t *chroma_user_desc)
{
    uint32_t i;
    void *target_ptr;
    tivx_vpac_ldc_bit_depth_conv_lut_params_t *lut_prms;
    vx_status status = (vx_status)VX_SUCCESS;

    if(NULL != luma_user_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&luma_user_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, luma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        lut_prms = (tivx_vpac_ldc_bit_depth_conv_lut_params_t *)target_ptr;

        settings->ylut_en = 1;
        settings->yin_bits = lut_prms->input_bits;
        settings->yout_bits = lut_prms->output_bits;

        for(i = 0; i < 513; i++) {
            settings->ylut[i]  = lut_prms->lut[i];  // subframe enable
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, luma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if(NULL != chroma_user_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&chroma_user_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, chroma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        lut_prms = (tivx_vpac_ldc_bit_depth_conv_lut_params_t *)target_ptr;

        settings->clut_en = 1;
        settings->cin_bits = lut_prms->input_bits;
        settings->cout_bits = lut_prms->output_bits;

        for(i = 0; i < 513; i++) {
            settings->clut[i]  = lut_prms->lut[i];  // subframe enable
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, chroma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    return (status);
}
