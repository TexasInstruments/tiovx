/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include "TI/tivx.h"
#include "TI/tda4x.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_viss.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"
#include "viss_top.h"

#define NO_SHIFT 0
#define UP_SHIFT 1
#define DOWN_SHIFT 2

#define VISS_MAX_PATH_SIZE 256

static tivx_target_kernel vx_vpac_viss_target_kernel = NULL;

static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacVissFreeMem(viss_config *prms);
static uint32_t tivxVpacVissFindFile(char *root_name, char *dir_name, char *substring, char *full_path);


static vx_status VX_CALLBACK tivxVpacVissProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_array_t *configuration_desc;
    tivx_obj_desc_array_t *ae_awb_result_desc;
    tivx_obj_desc_image_t *raw0_desc;
    tivx_obj_desc_image_t *raw1_desc;
    tivx_obj_desc_image_t *raw2_desc;
    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_array_t *h3a_aew_af_desc;
    viss_config *prms = NULL;
    tivx_vpac_viss_params_t *params;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW0_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        uint32_t size;

        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW0_IDX];
        raw1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW1_IDX];
        raw2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW2_IDX];
        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_Y12_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_UV12_C1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_Y8_R8_C2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_UV8_G8_C3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_S8_B8_C4_IDX];
        histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(viss_config) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        void *configuration_target_ptr;
        void *ae_awb_result_target_ptr;
        void *raw0_target_ptr;
        void *raw1_target_ptr = NULL;
        void *raw2_target_ptr = NULL;
        void *y12_target_ptr = NULL;
        void *uv12_c1_target_ptr = NULL;
        void *y8_r8_c2_target_ptr = NULL;
        void *uv8_g8_c3_target_ptr = NULL;
        void *s8_b8_c4_target_ptr = NULL;
        void *histogram_target_ptr = NULL;
        void *h3a_aew_af_target_ptr = NULL;

        configuration_target_ptr = tivxMemShared2TargetPtr(
          configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_heap_region);
        tivxMemBufferMap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        ae_awb_result_target_ptr = tivxMemShared2TargetPtr(
          ae_awb_result_desc->mem_ptr.shared_ptr, ae_awb_result_desc->mem_ptr.mem_heap_region);
        tivxMemBufferMap(ae_awb_result_target_ptr,
           ae_awb_result_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        raw0_target_ptr = tivxMemShared2TargetPtr(
          raw0_desc->mem_ptr[0].shared_ptr, raw0_desc->mem_ptr[0].mem_heap_region);
        tivxMemBufferMap(raw0_target_ptr,
           raw0_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        if( raw1_desc != NULL)
        {
            raw1_target_ptr = tivxMemShared2TargetPtr(
              raw1_desc->mem_ptr[0].shared_ptr, raw1_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(raw1_target_ptr,
               raw1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        if( raw2_desc != NULL)
        {
            raw2_target_ptr = tivxMemShared2TargetPtr(
              raw2_desc->mem_ptr[0].shared_ptr, raw2_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(raw2_target_ptr,
               raw2_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        if( y12_desc != NULL)
        {
            y12_target_ptr = tivxMemShared2TargetPtr(
              y12_desc->mem_ptr[0].shared_ptr, y12_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(y12_target_ptr,
               y12_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( uv12_c1_desc != NULL)
        {
            uv12_c1_target_ptr = tivxMemShared2TargetPtr(
              uv12_c1_desc->mem_ptr[0].shared_ptr, uv12_c1_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( y8_r8_c2_desc != NULL)
        {
            y8_r8_c2_target_ptr = tivxMemShared2TargetPtr(
              y8_r8_c2_desc->mem_ptr[0].shared_ptr, y8_r8_c2_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( uv8_g8_c3_desc != NULL)
        {
            uv8_g8_c3_target_ptr = tivxMemShared2TargetPtr(
              uv8_g8_c3_desc->mem_ptr[0].shared_ptr, uv8_g8_c3_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( s8_b8_c4_desc != NULL)
        {
            s8_b8_c4_target_ptr = tivxMemShared2TargetPtr(
              s8_b8_c4_desc->mem_ptr[0].shared_ptr, s8_b8_c4_desc->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( histogram_desc != NULL)
        {
            histogram_target_ptr = tivxMemShared2TargetPtr(
              histogram_desc->mem_ptr.shared_ptr, histogram_desc->mem_ptr.mem_heap_region);
            tivxMemBufferMap(histogram_target_ptr,
               histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( h3a_aew_af_desc != NULL)
        {
            h3a_aew_af_target_ptr = tivxMemShared2TargetPtr(
              h3a_aew_af_desc->mem_ptr.shared_ptr, h3a_aew_af_desc->mem_ptr.mem_heap_region);
            tivxMemBufferMap(h3a_aew_af_target_ptr,
               h3a_aew_af_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        params = (tivx_vpac_viss_params_t *)configuration_target_ptr;

        /* call kernel processing function */

        /* Read non-NUll input buffers (up to 3) */
        lse_reformat_in(raw0_desc, raw0_target_ptr, prms->buffer[0]);
        if (raw1_desc != NULL)
        {
            lse_reformat_in(raw1_desc, raw1_target_ptr, prms->buffer[1*2]);
        }
        if (raw2_desc != NULL)
        {
            lse_reformat_in(raw2_desc, raw2_target_ptr, prms->buffer[2*2]);
        }

        /* PROCESSING */

        /*                                  |-------------------->
         *                                  |        |----------->
         * RAWFE -> NSF4 -> GLBCE -> FLEXCFA -> FLEXCC -> EE ---->
         *     '--> H3A --->                         '--> HIST -->
         */

        /* RAWFE */
        if(0 == params->mux_h3a_in)
        {
            prms->rawfe_params.h3a_mux_sel = 2;
        }
        else if(1 == params->mux_h3a_in)
        {
            prms->rawfe_params.h3a_mux_sel = 1;
        }
        else if(2 == params->mux_h3a_in)
        {
            prms->rawfe_params.h3a_mux_sel = 0;
        }
        else
        {
            prms->rawfe_params.h3a_mux_sel = 3;
        }

        if ((VX_SUCCESS == status) && (NULL != histogram_desc))
        {
            prms->hist = histogram_target_ptr;
        }

        if ((VX_SUCCESS == status) && (NULL != h3a_aew_af_desc))
        {
            tivx_h3a_data_t *pH3a_buf = (tivx_h3a_data_t*)h3a_aew_af_target_ptr;
            pH3a_buf->aew_af_mode = params->mux_h3a_out;
            pH3a_buf->h3a_source_data = params->mux_h3a_in;
            prms->h3a = &pH3a_buf->data;
            h3a_aew_af_desc->num_items = 1;
        }

        status = vlab_hwa_process(VPAC_VISS_BASE_ADDRESS, "VPAC_VISS", sizeof(viss_config), &prms);

        /* Fill non-NULL output buffers (up to 7) */

        if( y12_desc != NULL)
        {
            lse_reformat_out(raw0_desc, y12_desc, y12_target_ptr, prms->buffer[3*2], 12);
        }
        if( uv12_c1_desc != NULL)
        {
            lse_reformat_out(raw0_desc, uv12_c1_desc, uv12_c1_target_ptr, prms->buffer[4*2], 12);
        }
        if( y8_r8_c2_desc != NULL)
        {
            uint16_t  out_y8_r8_c2_bit_align = 8;
            if(2 == params->mux_y8_r8_c2_out)
            {
                out_y8_r8_c2_bit_align = 12;
            }
            lse_reformat_out(raw0_desc, y8_r8_c2_desc, y8_r8_c2_target_ptr, prms->buffer[5*2], out_y8_r8_c2_bit_align);
        }
        if( uv8_g8_c3_desc != NULL)
        {
            uint16_t  out_uv8_g8_c3_bit_align = 8;
            if(2 == params->mux_uv8_g8_c3_out)
            {
                out_uv8_g8_c3_bit_align = 12;
            }
            lse_reformat_out(raw0_desc, uv8_g8_c3_desc, uv8_g8_c3_target_ptr, prms->buffer[6*2], out_uv8_g8_c3_bit_align);
        }
        if( s8_b8_c4_desc != NULL)
        {
            uint16_t  out_s8_b8_c4_bit_align = 8;
            if(2 == params->mux_s8_b8_c4_out)
            {
                out_s8_b8_c4_bit_align = 12;
            }
            lse_reformat_out(raw0_desc, s8_b8_c4_desc, s8_b8_c4_target_ptr, prms->buffer[7*2], out_s8_b8_c4_bit_align);
        }


        /* kernel processing function complete */

        tivxMemBufferUnmap(configuration_target_ptr,
           configuration_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(ae_awb_result_target_ptr,
           ae_awb_result_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        tivxMemBufferUnmap(raw0_target_ptr,
           raw0_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);

        if( raw1_desc != NULL)
        {
            tivxMemBufferUnmap(raw1_target_ptr,
               raw1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        if( raw2_desc != NULL)
        {
            tivxMemBufferUnmap(raw2_target_ptr,
               raw2_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);
        }

        if( y12_desc != NULL)
        {
            tivxMemBufferUnmap(y12_target_ptr,
               y12_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( uv12_c1_desc != NULL)
        {
            tivxMemBufferUnmap(uv12_c1_target_ptr,
               uv12_c1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( y8_r8_c2_desc != NULL)
        {
            tivxMemBufferUnmap(y8_r8_c2_target_ptr,
               y8_r8_c2_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( uv8_g8_c3_desc != NULL)
        {
            tivxMemBufferUnmap(uv8_g8_c3_target_ptr,
               uv8_g8_c3_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( s8_b8_c4_desc != NULL)
        {
            tivxMemBufferUnmap(s8_b8_c4_target_ptr,
               s8_b8_c4_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( histogram_desc != NULL)
        {
            tivxMemBufferUnmap(histogram_target_ptr,
               histogram_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

        if( h3a_aew_af_desc != NULL)
        {
            tivxMemBufferUnmap(h3a_aew_af_target_ptr,
               h3a_aew_af_desc->mem_size, VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }

    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_array_t *configuration_desc;
    tivx_obj_desc_array_t *ae_awb_result_desc;
    tivx_obj_desc_image_t *raw0_desc;
    tivx_obj_desc_image_t *raw1_desc;
    tivx_obj_desc_image_t *raw2_desc;
    tivx_obj_desc_image_t *y12_desc;
    tivx_obj_desc_image_t *uv12_c1_desc;
    tivx_obj_desc_image_t *y8_r8_c2_desc;
    tivx_obj_desc_image_t *uv8_g8_c3_desc;
    tivx_obj_desc_image_t *s8_b8_c4_desc;
    tivx_obj_desc_distribution_t *histogram_desc;
    tivx_obj_desc_array_t *h3a_aew_af_desc;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW0_IDX])
    )
    {
        status = VX_FAILURE;
    }
    
    if(VX_SUCCESS == status)
    {
        viss_config *prms = NULL;
        tivx_vpac_viss_params_t *params;

        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX];
        ae_awb_result_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];
        raw0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW0_IDX];
        raw1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW1_IDX];
        raw2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_RAW2_IDX];
        y12_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_Y12_IDX];
        uv12_c1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_UV12_C1_IDX];
        y8_r8_c2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_Y8_R8_C2_IDX];
        uv8_g8_c3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_UV8_G8_C3_IDX];
        s8_b8_c4_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_S8_B8_C4_IDX];
        histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_HISTOGRAM_IDX];
        h3a_aew_af_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_VISS_H3A_AEW_AF_IDX];

        prms = tivxMemAlloc(sizeof(viss_config), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            uint32_t width = raw0_desc->imagepatch_addr[0].dim_x;
            uint32_t height = raw0_desc->imagepatch_addr[0].dim_y;
            uint32_t i;

            memset(prms, 0, sizeof(viss_config));

            prms->buffer_size = (width * height) * 2;
            
            if(NULL != raw0_desc)
            {
                prms->buffer[0] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[0])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != raw1_desc))
            {
                prms->buffer[1*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[1*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != raw2_desc))
            {
                prms->buffer[2*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[2*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != y12_desc))
            {
                prms->buffer[3*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[3*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != uv12_c1_desc))
            {
                prms->buffer[4*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[4*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != y8_r8_c2_desc))
            {
                prms->buffer[5*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[5*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != uv8_g8_c3_desc))
            {
                prms->buffer[6*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[6*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if ((VX_SUCCESS == status) && (NULL != s8_b8_c4_desc))
            {
                prms->buffer[7*2] = tivxMemAlloc(prms->buffer_size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->buffer[7*2])
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                DIR *dir;
                void *configuration_target_ptr;
                void *ae_awb_result_target_ptr;

                configuration_target_ptr = tivxMemShared2TargetPtr(
                    configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_heap_region);
                ae_awb_result_target_ptr = tivxMemShared2TargetPtr(
                    ae_awb_result_desc->mem_ptr.shared_ptr, ae_awb_result_desc->mem_ptr.mem_heap_region);

                tivxMemBufferMap(configuration_target_ptr, configuration_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
                tivxMemBufferMap(ae_awb_result_target_ptr, ae_awb_result_desc->mem_size,
                    VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

                params = (tivx_vpac_viss_params_t *)configuration_target_ptr;

                /* Check for top level directory */
                dir = opendir(params->sensor_name);
                if (NULL != dir)
                {
                    closedir(dir);
                }
                else if (ENOENT == errno)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Directory does not exist: %s\n", params->sensor_name);
                    status = VX_FAILURE;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Directory is not accessible: %s\n", params->sensor_name);
                    status = VX_FAILURE;
                }
            }

            if (VX_SUCCESS == status)
            {
                char path_rawfe[] =   "/Rawfe_tasks/";
                char path_flexcfa[] = "/FlexCFA_tasks/";
                char path_flexcc[] =  "/FlexCC_tasks/";
                char temp_path[VISS_MAX_PATH_SIZE];
                char temp_name[VISS_MAX_PATH_SIZE];
  
                FILE *h3a_config;
                int32_t bits = 12;
                int32_t w, h;

                /*                                  |-------------------->
                 *                                  |        |----------->
                 * RAWFE -> NSF4 -> GLBCE -> FLEXCFA -> FLEXCC -> EE ---->
                 *     '--> H3A --->                         '--> HIST -->
                 */

                /* RAWFE */
                strncpy(temp_path, params->sensor_name, VISS_MAX_PATH_SIZE);
                strncat(temp_path, path_rawfe, sizeof(path_rawfe));
                if(0 != tivxVpacVissFindFile(params->sensor_name, path_rawfe, "cfg_rawfe_master", temp_name))
                {
                    read_rawfe_cfg(temp_name, temp_path, &prms->rawfe_params);
                    prms->rawfe_params.width = width;
                    prms->rawfe_params.height = height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Rawfe config file not found\n");
                    status = VX_FAILURE;
                }

                /* H3A */
                if( h3a_aew_af_desc != NULL)
                {
                    int32_t num_aew_windows, ae_size, af_pad;

                    if(0 != tivxVpacVissFindFile(params->sensor_name, "/H3a_tasks/", "cfg_h3a_master", temp_name))
                    {
                        h3a_config = fopen(temp_name, "r");
                        /* Need to read the width and height from file first to align rest of settings config */
                        read_paramval(h3a_config);
                        read_paramval(h3a_config);
                        read_h3a_configfile(h3a_config, &prms->h3a_params);
                        fclose(h3a_config);

                        /* Compute AEW buffer size */
                        num_aew_windows = (prms->h3a_params.aewwin1_WINVC+1)*(prms->h3a_params.aewwin1_WINHC);
                        ae_size = 0;
                        for (i = 0; i < num_aew_windows; i++)
                        {
                            if ( (prms->h3a_params.aew_cfg_AEFMT == 0) || (prms->h3a_params.aew_cfg_AEFMT == 1))
                            {
                                ae_size += 8;
                            }
                            else
                            {
                                ae_size += 4;
                            }
                            if (i%8 == 7 && i > 0)
                            {
                                ae_size += 4;
                            }
                            if (i%prms->h3a_params.aewwin1_WINHC == (prms->h3a_params.aewwin1_WINHC -1) && ae_size%8 == 4 && i != (num_aew_windows-1))
                            {
                                ae_size += 4;
                            }
                        }
                        if (num_aew_windows%8 != 0)
                        {
                            ae_size += 4;
                        }
                        if (ae_size %8 != 0)
                        {
                            ae_size+= 4;
                        }
                        prms->aew_buffer_size = ae_size * sizeof(uint32_t);

                        /* Compute AF buffer size */
                        if (prms->h3a_params.pcr_AF_VF_EN)
                        {
                            prms->af_buffer_size = (16U * (prms->h3a_params.afpax2_PAXHC * prms->h3a_params.afpax2_PAXVC)) * sizeof(uint32_t);
                        }
                        else
                        {
                            af_pad = 0;
                            if (1U == (prms->h3a_params.afpax2_PAXHC%2))
                            {
                                af_pad = 4*prms->h3a_params.afpax2_PAXVC;
                            }
                            prms->af_buffer_size = ((12 * (prms->h3a_params.afpax2_PAXHC * prms->h3a_params.afpax2_PAXVC)) + af_pad) * sizeof(uint32_t);
                        }

                        if (VX_SUCCESS == status)
                        {
                            prms->h3a_buffer_size = prms->af_buffer_size;
                            if(prms->aew_buffer_size > prms->h3a_buffer_size)
                            {
                                prms->h3a_buffer_size = prms->aew_buffer_size;
                            }
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "H3A config file not found\n");
                        status = VX_FAILURE;
                    }
                }

                /* NSF4 */ 
                if(0 == params->bypass_nsf4)
                {
                    if(0 != tivxVpacVissFindFile(params->sensor_name, "/Nsf4v_tasks/", "cfg_nsf4_master", temp_name))
                    {
                        nsf4_read_parameters(temp_name, &prms->nsf4_params);
                        nsf4_check_parameters(&prms->nsf4_params);
                        prms->nsf4_params.iw = width;
                        prms->nsf4_params.ih = height;
                    }
                    else
                    {
                        prms->bypass_nsf4 = 1;
                        VX_PRINT(VX_ZONE_WARNING, "NSF4 config file not found, bypassing NSF4 module\n");
                    }
                }
                else
                {
                    prms->bypass_nsf4 = 1;
                }

                /* GLBCE */
                /* Missing for now */

                /* FLEXCFA */
                strncpy(temp_path, params->sensor_name, VISS_MAX_PATH_SIZE);
                strncat(temp_path, path_flexcfa, sizeof(path_flexcfa));
                if(0 != tivxVpacVissFindFile(params->sensor_name, path_flexcfa, "cfg_flexcfa_master", temp_name))
                {
                    flxd_read_parameters(temp_name, &prms->flexcfa_params, &w, &h, &bits, temp_path);
                    prms->flexcfa_params.imgWidth = width;
                    prms->flexcfa_params.imgHeight = height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "FlexCFA config file not found\n");
                    status = VX_FAILURE;
                }

                /* FLEXCC */
                if( ( NULL != y12_desc ) ||
                   (( NULL != uv12_c1_desc )   && (2 != params->mux_uv12_c1_out)) ||
                   (( NULL != y8_r8_c2_desc )  && (2 != params->mux_y8_r8_c2_out)) ||
                   (( NULL != uv8_g8_c3_desc ) && (2 != params->mux_uv8_g8_c3_out)) ||
                   (( NULL != s8_b8_c4_desc )  && (2 != params->mux_s8_b8_c4_out)) ||
                    ( NULL != histogram_desc))
                {
                    strncpy(temp_path, params->sensor_name, VISS_MAX_PATH_SIZE);
                    strncat(temp_path, path_flexcc, sizeof(path_flexcc));
                    if(0 != tivxVpacVissFindFile(params->sensor_name, path_flexcc, "cfg_flexcc_master", temp_name))
                    {
                        flexcc_read_parameters(temp_name, &prms->flexcc_params, &w, &h, temp_path);
                        prms->flexcc_params.inWidth = width;
                        prms->flexcc_params.inHeight = height;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "FlexCC config file not found\n");
                        status = VX_FAILURE;
                    }
                }
                else
                {
                    prms->bypass_cc = 1;
                }

                /* EE */
                if(2 != params->mux_ee_port)
                {
                    if((( NULL != y12_desc )       && (0 == params->mux_ee_port)) ||
                       (( NULL != y8_r8_c2_desc )  && (0 == params->mux_y8_r8_c2_out) && (1 == params->mux_ee_port)))
                    {
                        if(0 != tivxVpacVissFindFile(params->sensor_name, "/edgeEnhancer_tasks/", "cfg_ee_master", temp_name))
                        {
                            read_ee_cfg(temp_name, &prms->ee_params);
                            prms->ee_params.width = width;
                            prms->ee_params.height = height;
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Edge Enhancer config file not found\n");
                            status = VX_FAILURE;
                        }

                        if(0 != tivxVpacVissFindFile(params->sensor_name, "/edgeEnhancer_tasks/", "lut_ee", temp_name))
                        {
                            read_lutfile(temp_name, prms->ee_params.yee_table_s13, 4096);
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Edge Enhancer lut file not found\n");
                            status = VX_FAILURE;
                        }
                    }
                    else
                    {
                        prms->bypass_ee = 1;
                    }
                }
                else
                {
                    prms->bypass_ee = 1;
                }
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(viss_config));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacVissFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    if ( num_params != TIVX_KERNEL_VPAC_VISS_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_VISS_RAW0_IDX])
    )
    {
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        viss_config *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(viss_config) == size))
        {
            tivxVpacVissFreeMem(prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacVissControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelVpacViss(void)
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_IPU1_0) || (self_cpu == TIVX_CPU_ID_IPU1_1))
    {
        strncpy(target_name, TIVX_TARGET_VPAC_VISS1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_VISS_NAME,
                            target_name,
                            tivxVpacVissProcess,
                            tivxVpacVissCreate,
                            tivxVpacVissDelete,
                            tivxVpacVissControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelVpacViss(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_viss_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_vpac_viss_target_kernel = NULL;
    }
}

static void tivxVpacVissFreeMem(viss_config *prms)
{
    if (NULL != prms)
    {
        uint32_t i;

        for (i = 0; i < NUM_BUFFERS; i++)
        {
            if (NULL != prms->buffer[i*2])
            {
                tivxMemFree(prms->buffer[i*2], prms->buffer_size, TIVX_MEM_EXTERNAL);
                prms->buffer[i*2] = NULL;
            }
        }

        tivxMemFree(prms, sizeof(viss_config), TIVX_MEM_EXTERNAL);
    }
}

static uint32_t tivxVpacVissFindFile(char *root_name, char *dir_name, char *substring, char *full_path)
{
    DIR *dir;
    struct dirent *ent;
    uint32_t found = 0;
    
    strncpy(full_path, root_name, VISS_MAX_PATH_SIZE);
    strncat(full_path, dir_name, VISS_MAX_PATH_SIZE-1);

    if ((dir = opendir (full_path)) != NULL)
    {
        char *ptr;
        while ((ent = readdir (dir)) != NULL)
        {
            ptr = strstr(ent->d_name, substring);
            if (NULL != ptr)
            {
                strncat(full_path, ent->d_name, VISS_MAX_PATH_SIZE-1);
                found = 1;
                break;
            }
        }
        closedir (dir);
    }

    return found;
}
