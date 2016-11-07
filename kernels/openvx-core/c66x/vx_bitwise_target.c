/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_bitwise.h>
#include <TI/tivx_target_kernel.h>

static vx_uint32 bitwiseComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t* addr)
{
    return (addr->stride_y * y / addr->step_y) +
           (addr->stride_x * x / addr->step_x);
}

static void* VX_API_CALL bitwiseFormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && x < addr->dim_x && y < addr->dim_y)
    {
        vx_uint32 offset = bitwiseComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}


static tivx_target_kernel vx_bitwise_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelBitwiseProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 y, x, width = 0, height = 0;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src_addr, *dst_addr;

    if ((num_params != 2U) || (NULL == obj_desc[0]) || (NULL == obj_desc[1]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_OUT_IMG_IDX];

        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(src_desc->mem_ptr[0].target_ptr, src_desc->mem_size[0],
            src_desc->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src_desc->mem_ptr[0].target_ptr +
            bitwiseComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0]));
        dst_addr = (uint8_t *)((uint32_t)dst_desc->mem_ptr[0].target_ptr +
            bitwiseComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        height = src_desc->imagepatch_addr[0].dim_y;
        width = src_desc->imagepatch_addr[0].dim_x;

        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {
                vx_uint8 *src = bitwiseFormatImagePatchAddress2d(
                    src_addr, x, y, &src_desc->imagepatch_addr[0]);
                vx_uint8 *dst = bitwiseFormatImagePatchAddress2d(
                    dst_addr, x, y, &dst_desc->imagepatch_addr[0]);

                *dst = ~*src;
            }
        }

        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBitwiseCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBitwise()
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

        vx_bitwise_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_NOT,
            target_name,
            tivxKernelBitwiseProcess,
            tivxKernelBitwiseCreate,
            tivxKernelBitwiseDelete,
            tivxKernelBitwiseControl);
    }
}


void tivxRemoveTargetKernelBitwise()
{
    tivxRemoveTargetKernel(vx_bitwise_target_kernel);
}
