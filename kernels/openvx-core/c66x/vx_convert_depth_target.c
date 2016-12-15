/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_convert_depth.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_convert_depth_target_kernel = NULL;

/* Work on this section */
static vx_status tivxKernelConvertDepth(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    tivx_obj_desc_scalar_t *sc_desc[2];
    uint32_t i;
    void *src_addr, *dst_addr;
    vx_rectangle_t rect;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t overflow_policy;

    if (num_params != TIVX_KERNEL_CONVERT_DEPTH_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CONVERT_DEPTH_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        src_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_OUT_IMG_IDX];
        sc_desc[0] = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_IN0_SCALAR_IDX];
        sc_desc[1] = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_CONVERT_DEPTH_IN1_SCALAR_IDX];

        src_desc->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0U].shared_ptr, src_desc->mem_ptr[0U].mem_type);

        dst_desc->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0U].shared_ptr, dst_desc->mem_ptr[0U].mem_type);

        tivxMemBufferMap(src_desc->mem_ptr[0U].target_ptr, src_desc->mem_size[0],
            src_desc->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr, dst_desc->mem_size[0],
            dst_desc->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src image */
        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));

        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uint32_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        vxlib_src.dim_x = src_desc->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == src_desc->format)
        {
            vxlib_src.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_src.data_type = VXLIB_INT16;
        }

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;

        if (VX_DF_IMAGE_U8 == dst_desc->format)
        {
            vxlib_dst.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_dst.data_type = VXLIB_INT16;
        }

        if (VX_CONVERT_POLICY_SATURATE == sc_desc[0]->data.enm)
        {
            overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
        }
        else
        {
            overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
        }

        if (VXLIB_INT16 == vxlib_dst.data_type)
        {
            status = VXLIB_convertDepth_8to16((uint8_t *)src_addr,
                        &vxlib_src, (int16_t *)dst_addr, &vxlib_dst, sc_desc[1]->data.s32);
        }
        else
        {
            status = VXLIB_convertDepth_16to8((int16_t *)src_addr,
                        &vxlib_src, (uint8_t *)dst_addr, &vxlib_dst, sc_desc[1]->data.s32, overflow_policy);
        }



        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src_desc->mem_ptr[0U].target_ptr,
            src_desc->mem_size[0], src_desc->mem_ptr[0U].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0U].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0U].mem_type,
            VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvertDepthProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelConvertDepth(kernel, obj_desc, num_params, VX_KERNEL_CONVERTDEPTH);

    return (status);
}

void tivxAddTargetKernelConvertDepth()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_convert_depth_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CONVERTDEPTH,
            target_name,
            tivxKernelConvertDepthProcess,
            tivxKernelConvertDepthCreate,
            tivxKernelConvertDepthDelete,
            tivxKernelConvertDepthControl,
            NULL);
    }
}


void tivxRemoveTargetKernelConvertDepth()
{
    tivxRemoveTargetKernel(vx_convert_depth_target_kernel);
}

