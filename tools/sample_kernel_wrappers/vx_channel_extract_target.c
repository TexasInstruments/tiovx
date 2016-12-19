/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_channel_extract.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_channel_extract_target_kernel = NULL;

vx_status tivxChannelExtract(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *in_desc;
    tivx_obj_desc_scalar_t *channel_desc;
    tivx_obj_desc_image_t *out_desc;
    
    if ( num_params != TIVX_KERNEL_CHANNEL_EXTRACT_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        vx_enum channel_value;
        
        in_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_IN_IDX];
        channel_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_CHANNEL_IDX];
        out_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CHANNEL_EXTRACT_OUT_IDX];
        
        out_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          out_desc->mem_ptr[0].shared_ptr, out_desc->mem_ptr[0].mem_type);
        
        tivxMemBufferMap(out_desc->mem_ptr[0].target_ptr,
           out_desc->mem_size[0], out_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        
        channel_value = channel_desc->data.enm;
        
        /* call kernel processing function */
        
        
        
        /* kernel processing function complete */
        
        tivxMemBufferUnmap(out_desc->mem_ptr[0].target_ptr,
           out_desc->mem_size[0], out_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        
        
    }
    
    return status;
}

vx_status tivxChannelExtractCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

vx_status tivxChannelExtractDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

vx_status tivxChannelExtractControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    
    return status;
}

void tivxAddTargetKernelChannelExtract()
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
        vx_channel_extract_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_CHANNEL_EXTRACT,
                            target_name,
                            tivxChannelExtract,
                            tivxChannelExtractCreate,
                            tivxChannelExtractDelete,
                            tivxChannelExtractControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelChannelExtract()
{
    vx_status status = VX_SUCCESS;
    
    status = tivxRemoveTargetKernel(vx_channel_extract_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_channel_extract_target_kernel = NULL;
    }
}


