/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_channel_combine.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_channel_combine_target_kernel = NULL;

vx_status tivxChannelCombine(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc;
    tivx_obj_desc_image_t *src1_desc;
    tivx_obj_desc_image_t *src2_desc;
    tivx_obj_desc_image_t *src3_desc;
    tivx_obj_desc_image_t *dst_desc;
    uint16_t plane_idx;
    
    if ( num_params != TIVX_KERNEL_CHANNEL_COMBINE_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC0_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC1_IDX];
        src2_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC2_IDX];
        src3_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_SRC3_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_COMBINE_DST_IDX];
        
        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);
        if( src2_desc != NULL)
        {
            src2_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              src2_desc->mem_ptr[0].shared_ptr, src2_desc->mem_ptr[0].mem_type);
        }
        if( src3_desc != NULL)
        {
            src3_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
              src3_desc->mem_ptr[0].shared_ptr, src3_desc->mem_ptr[0].mem_type);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            dst_desc->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(
              dst_desc->mem_ptr[plane_idx].shared_ptr, dst_desc->mem_ptr[plane_idx].mem_type);
        }
        
        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr,
           src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr,
           src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        if( src2_desc != NULL)
        {
            tivxMemBufferMap(src2_desc->mem_ptr[0].target_ptr,
               src2_desc->mem_size[0], src2_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( src3_desc != NULL)
        {
            tivxMemBufferMap(src3_desc->mem_ptr[0].target_ptr,
               src3_desc->mem_size[0], src3_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxMemBufferMap(dst_desc->mem_ptr[plane_idx].target_ptr,
               dst_desc->mem_size[plane_idx], dst_desc->mem_ptr[plane_idx].mem_type,
                VX_WRITE_ONLY);
        }
        
        
        /* call kernel processing function */
        
        
        
        /* kernel processing function complete */
        
        tivxMemBufferUnmap(src0_desc->mem_ptr[0].target_ptr,
           src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src1_desc->mem_ptr[0].target_ptr,
           src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        if( src2_desc != NULL)
        {
            tivxMemBufferUnmap(src2_desc->mem_ptr[0].target_ptr,
               src2_desc->mem_size[0], src2_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        if( src3_desc != NULL)
        {
            tivxMemBufferUnmap(src3_desc->mem_ptr[0].target_ptr,
               src3_desc->mem_size[0], src3_desc->mem_ptr[0].mem_type,
                VX_READ_ONLY);
        }
        for(plane_idx=0; plane_idx<dst_desc->planes; plane_idx++)
        {
            tivxMemBufferUnmap(dst_desc->mem_ptr[plane_idx].target_ptr,
               dst_desc->mem_size[plane_idx], dst_desc->mem_ptr[plane_idx].mem_type,
                VX_WRITE_ONLY);
        }
        
        
    }
    
    return status;
}

vx_status tivxChannelCombineCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

vx_status tivxChannelCombineDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

vx_status tivxChannelCombineControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

void tivxAddTargetKernelChannelCombine()
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    
    self_cpu = tivxGetSelfCpuId();
    
    if ( self_cpu == TIVX_CPU_ID_DSP1 )
    {
        strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    if ( self_cpu == TIVX_CPU_ID_DSP2 )
    {
        strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }
    
    if (status == VX_SUCCESS)
    {
        vx_channel_combine_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_CHANNEL_COMBINE,
                            target_name,
                            tivxChannelCombine,
                            tivxChannelCombineCreate,
                            tivxChannelCombineDelete,
                            tivxChannelCombineControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelChannelCombine()
{
    vx_status status = VX_SUCCESS;
    
    status = tivxRemoveTargetKernel(vx_channel_combine_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_channel_combine_target_kernel = NULL;
    }
}


