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
#include <tivx_kernel_magnitude.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_magnitude_target_kernel = NULL;

vx_status VX_CALLBACK tivxMagnitude(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;

    if ((num_params != 3U) || (NULL == obj_desc[0U]) || (NULL == obj_desc[1U]) ||
        (NULL == obj_desc[2U]))
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_MAGNITUDE_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_MAGNITUDE_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_MAGNITUDE_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr,
            src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr,
            src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src0.dim_x = src0_desc->imagepatch_addr[0U].dim_x;
        vxlib_src0.dim_y = src0_desc->imagepatch_addr[0U].dim_y;
        vxlib_src0.stride_y = src0_desc->imagepatch_addr[0U].stride_y;
        vxlib_src0.data_type = VXLIB_INT16;

        vxlib_src1.dim_x = src1_desc->imagepatch_addr[0U].dim_x;
        vxlib_src1.dim_y = src1_desc->imagepatch_addr[0U].dim_y;
        vxlib_src1.stride_y = src1_desc->imagepatch_addr[0U].stride_y;
        vxlib_src1.data_type = VXLIB_INT16;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        vxlib_dst.data_type = VXLIB_INT16;

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src0_desc->valid_roi;

        src0_addr = (uint8_t *)((uint32_t)src0_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src0_desc->imagepatch_addr[0U]));
        src1_addr = (uint8_t *)((uint32_t)src1_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src1_desc->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uint32_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        status = VXLIB_magnitude_i16s_i16s_o16s(
                (int16_t *)src0_addr, &vxlib_src0, (int16_t *)src1_addr,
                &vxlib_src1, (int16_t *)dst_addr, &vxlib_dst);

        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src0_desc->mem_ptr[0].target_ptr,
            src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src1_desc->mem_ptr[0].target_ptr,
            src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

    }

    return (status);
}

vx_status VX_CALLBACK tivxMagnitudeCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxMagnitudeDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK tivxMagnitudeControl(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelMagnitude()
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

        vx_magnitude_target_kernel = tivxAddTargetKernel(
                    VX_KERNEL_MAGNITUDE,
                    target_name,
                    tivxMagnitude,
                    tivxMagnitudeCreate,
                    tivxMagnitudeDelete,
                    tivxMagnitudeControl,
                    NULL);
    }
}

void tivxRemoveTargetKernelMagnitude()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_magnitude_target_kernel);

    if (VX_SUCCESS == status)
    {
        vx_magnitude_target_kernel = NULL;
    }
}

