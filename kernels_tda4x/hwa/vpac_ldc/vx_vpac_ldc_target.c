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
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_ldc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"

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
static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_array_t *configuration_desc;
    tivx_obj_desc_array_t *region_params_desc;
    tivx_obj_desc_image_t *mesh_table_desc;
    tivx_obj_desc_matrix_t *warp_matrix_desc;
    tivx_obj_desc_lut_t *out_2_luma_lut_desc;
    tivx_obj_desc_lut_t *out_3_chroma_lut_desc;
    tivx_obj_desc_array_t *bandwidth_params_desc;
    tivx_obj_desc_image_t *in_luma_or_422_desc;
    tivx_obj_desc_image_t *in_chroma_desc;
    tivx_obj_desc_image_t *out_0_luma_or_422_desc;
    tivx_obj_desc_image_t *out_1_chroma_desc;
    tivx_obj_desc_image_t *out_2_luma_or_422_desc;
    tivx_obj_desc_image_t *out_3_chroma_desc;
    tivx_obj_desc_scalar_t *error_status_desc;
    
    if ( num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        vx_uint32 error_status_value;
        
        configuration_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
        region_params_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PARAMS_IDX];
        mesh_table_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_TABLE_IDX];
        warp_matrix_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
        out_2_luma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_LUT_IDX];
        out_3_chroma_lut_desc = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_LUT_IDX];
        bandwidth_params_desc = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_BANDWIDTH_PARAMS_IDX];
        in_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_LUMA_OR_422_IDX];
        in_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_IN_CHROMA_IDX];
        out_0_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_0_LUMA_OR_422_IDX];
        out_1_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_1_CHROMA_IDX];
        out_2_luma_or_422_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_2_LUMA_OR_422_IDX];
        out_3_chroma_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_OUT_3_CHROMA_IDX];
        error_status_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_VPAC_LDC_ERROR_STATUS_IDX];
        
        configuration_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
          configuration_desc->mem_ptr.shared_ptr, configuration_desc->mem_ptr.mem_type);
        region_params_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
          region_params_desc->mem_ptr.shared_ptr, region_params_desc->mem_ptr.mem_type);
        if( mesh_table_desc != NULL)
        {
            mesh_table_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              mesh_table_desc->mem_ptr[0].shared_ptr, mesh_table_desc->mem_ptr[0].mem_type);
        }
        if( warp_matrix_desc != NULL)
        {
            warp_matrix_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
              warp_matrix_desc->mem_ptr.shared_ptr, warp_matrix_desc->mem_ptr.mem_type);
        }
        if( out_2_luma_lut_desc != NULL)
        {
            out_2_luma_lut_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
              out_2_luma_lut_desc->mem_ptr.shared_ptr, out_2_luma_lut_desc->mem_ptr.mem_type);
        }
        if( out_3_chroma_lut_desc != NULL)
        {
            out_3_chroma_lut_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
              out_3_chroma_lut_desc->mem_ptr.shared_ptr, out_3_chroma_lut_desc->mem_ptr.mem_type);
        }
        if( bandwidth_params_desc != NULL)
        {
            bandwidth_params_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
              bandwidth_params_desc->mem_ptr.shared_ptr, bandwidth_params_desc->mem_ptr.mem_type);
        }
        if( in_luma_or_422_desc != NULL)
        {
            in_luma_or_422_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              in_luma_or_422_desc->mem_ptr[0].shared_ptr, in_luma_or_422_desc->mem_ptr[0].mem_type);
        }
        if( in_chroma_desc != NULL)
        {
            in_chroma_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              in_chroma_desc->mem_ptr[0].shared_ptr, in_chroma_desc->mem_ptr[0].mem_type);
        }
        if( out_0_luma_or_422_desc != NULL)
        {
            out_0_luma_or_422_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              out_0_luma_or_422_desc->mem_ptr[0].shared_ptr, out_0_luma_or_422_desc->mem_ptr[0].mem_type);
        }
        if( out_1_chroma_desc != NULL)
        {
            out_1_chroma_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              out_1_chroma_desc->mem_ptr[0].shared_ptr, out_1_chroma_desc->mem_ptr[0].mem_type);
        }
        if( out_2_luma_or_422_desc != NULL)
        {
            out_2_luma_or_422_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              out_2_luma_or_422_desc->mem_ptr[0].shared_ptr, out_2_luma_or_422_desc->mem_ptr[0].mem_type);
        }
        if( out_3_chroma_desc != NULL)
        {
            out_3_chroma_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              out_3_chroma_desc->mem_ptr[0].shared_ptr, out_3_chroma_desc->mem_ptr[0].mem_type);
        }
        
        tivxMemBufferMap(configuration_desc->mem_ptr.target_ptr,
           configuration_desc->mem_size, configuration_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(region_params_desc->mem_ptr.target_ptr,
           region_params_desc->mem_size, region_params_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        if( mesh_table_desc != NULL)
        {
            tivxMemBufferMap(mesh_table_desc->mem_ptr[0].target_ptr,
               mesh_table_desc->mem_size[0], mesh_table_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( warp_matrix_desc != NULL)
        {
            tivxMemBufferMap(warp_matrix_desc->mem_ptr.target_ptr,
               warp_matrix_desc->mem_size, warp_matrix_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( out_2_luma_lut_desc != NULL)
        {
            tivxMemBufferMap(out_2_luma_lut_desc->mem_ptr.target_ptr,
               out_2_luma_lut_desc->mem_size, out_2_luma_lut_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( out_3_chroma_lut_desc != NULL)
        {
            tivxMemBufferMap(out_3_chroma_lut_desc->mem_ptr.target_ptr,
               out_3_chroma_lut_desc->mem_size, out_3_chroma_lut_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( bandwidth_params_desc != NULL)
        {
            tivxMemBufferMap(bandwidth_params_desc->mem_ptr.target_ptr,
               bandwidth_params_desc->mem_size, bandwidth_params_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( in_luma_or_422_desc != NULL)
        {
            tivxMemBufferMap(in_luma_or_422_desc->mem_ptr[0].target_ptr,
               in_luma_or_422_desc->mem_size[0], in_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( in_chroma_desc != NULL)
        {
            tivxMemBufferMap(in_chroma_desc->mem_ptr[0].target_ptr,
               in_chroma_desc->mem_size[0], in_chroma_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( out_0_luma_or_422_desc != NULL)
        {
            tivxMemBufferMap(out_0_luma_or_422_desc->mem_ptr[0].target_ptr,
               out_0_luma_or_422_desc->mem_size[0], out_0_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_1_chroma_desc != NULL)
        {
            tivxMemBufferMap(out_1_chroma_desc->mem_ptr[0].target_ptr,
               out_1_chroma_desc->mem_size[0], out_1_chroma_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_2_luma_or_422_desc != NULL)
        {
            tivxMemBufferMap(out_2_luma_or_422_desc->mem_ptr[0].target_ptr,
               out_2_luma_or_422_desc->mem_size[0], out_2_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_3_chroma_desc != NULL)
        {
            tivxMemBufferMap(out_3_chroma_desc->mem_ptr[0].target_ptr,
               out_3_chroma_desc->mem_size[0], out_3_chroma_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        
        
        /* call kernel processing function */
        
        /* < DEVELOPER_TODO: Add target kernel processing code here > */
        
        /* kernel processing function complete */
        
        tivxMemBufferUnmap(configuration_desc->mem_ptr.target_ptr,
           configuration_desc->mem_size, configuration_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(region_params_desc->mem_ptr.target_ptr,
           region_params_desc->mem_size, region_params_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        if( mesh_table_desc != NULL)
        {
            tivxMemBufferUnmap(mesh_table_desc->mem_ptr[0].target_ptr,
               mesh_table_desc->mem_size[0], mesh_table_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( warp_matrix_desc != NULL)
        {
            tivxMemBufferUnmap(warp_matrix_desc->mem_ptr.target_ptr,
               warp_matrix_desc->mem_size, warp_matrix_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( out_2_luma_lut_desc != NULL)
        {
            tivxMemBufferUnmap(out_2_luma_lut_desc->mem_ptr.target_ptr,
               out_2_luma_lut_desc->mem_size, out_2_luma_lut_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( out_3_chroma_lut_desc != NULL)
        {
            tivxMemBufferUnmap(out_3_chroma_lut_desc->mem_ptr.target_ptr,
               out_3_chroma_lut_desc->mem_size, out_3_chroma_lut_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( bandwidth_params_desc != NULL)
        {
            tivxMemBufferUnmap(bandwidth_params_desc->mem_ptr.target_ptr,
               bandwidth_params_desc->mem_size, bandwidth_params_desc->mem_ptr.mem_type,
                VX_READ_ONLY);
        }
        if( in_luma_or_422_desc != NULL)
        {
            tivxMemBufferUnmap(in_luma_or_422_desc->mem_ptr[0].target_ptr,
               in_luma_or_422_desc->mem_size[0], in_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( in_chroma_desc != NULL)
        {
            tivxMemBufferUnmap(in_chroma_desc->mem_ptr[0].target_ptr,
               in_chroma_desc->mem_size[0], in_chroma_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( out_0_luma_or_422_desc != NULL)
        {
            tivxMemBufferUnmap(out_0_luma_or_422_desc->mem_ptr[0].target_ptr,
               out_0_luma_or_422_desc->mem_size[0], out_0_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_1_chroma_desc != NULL)
        {
            tivxMemBufferUnmap(out_1_chroma_desc->mem_ptr[0].target_ptr,
               out_1_chroma_desc->mem_size[0], out_1_chroma_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_2_luma_or_422_desc != NULL)
        {
            tivxMemBufferUnmap(out_2_luma_or_422_desc->mem_ptr[0].target_ptr,
               out_2_luma_or_422_desc->mem_size[0], out_2_luma_or_422_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        if( out_3_chroma_desc != NULL)
        {
            tivxMemBufferUnmap(out_3_chroma_desc->mem_ptr[0].target_ptr,
               out_3_chroma_desc->mem_size[0], out_3_chroma_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        
        error_status_desc->data.u32 = error_status_value;
        
    }
    
    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    /* < DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating */
    /*                   local memory buffers, one time initialization, etc) > */
    
    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    /* < DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing */
    /*                   local memory buffers, etc) > */
    
    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    /* < DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands */
    /*                   the user can call to modify the processing of the kernel at run-time) > */
    
    return status;
}

void tivxAddTargetKernelVpacLdc()
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    
    self_cpu = tivxGetSelfCpuId();
    
    if ( self_cpu == TIVX_CPU_ID_IPU1_0 )
    {
        strncpy(target_name, TIVX_TARGET_IPU1_0, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }
    
    if (status == VX_SUCCESS)
    {
        vx_vpac_ldc_target_kernel = tivxAddTargetKernel(
                            TIVX_KERNEL_VPAC_LDC,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
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


