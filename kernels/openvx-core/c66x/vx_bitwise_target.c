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
#include <tivx_kernel_bitwise.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>

static tivx_target_kernel vx_bitwise_not_target_kernel = NULL;
static tivx_target_kernel vx_bitwise_and_target_kernel = NULL;
static tivx_target_kernel vx_bitwise_or_target_kernel = NULL;
static tivx_target_kernel vx_bitwise_xor_target_kernel = NULL;

typedef VXLIB_STATUS tivxBitwise_fxn(
    const uint8_t *src0, const VXLIB_bufParams2D_t *src0_prms,
    const uint8_t *src1, const VXLIB_bufParams2D_t *src1_prms,
    uint8_t *dst, const VXLIB_bufParams2D_t *dst_prms);

static inline vx_uint32 bitwiseComputePatchOffset(
    vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    return (addr->stride_y * (y / addr->step_y)) +
           (addr->stride_x * (x / addr->step_x));
}

static vx_status tivxKernelBitwiseProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    tivxBitwise_fxn bitwise_fxn)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    VXLIB_bufParams2D_t vlib_src0, vlib_src1, vlib_dst;

    if ((num_params != 3U) || (NULL == obj_desc[0U]) || (NULL == obj_desc[1U]) ||
        (NULL == obj_desc[2U]))
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           three buffers */
        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        /* Map all three buffers, which invalidates the cache */
        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr,
            src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr,
            src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vlib_src0.dim_x = src0_desc->imagepatch_addr[0U].dim_x;
        vlib_src0.dim_y = src0_desc->imagepatch_addr[0U].dim_y;
        vlib_src0.stride_y = src0_desc->imagepatch_addr[0U].stride_y;
        vlib_src0.data_type = VXLIB_UINT8;

        vlib_src1.dim_x = src1_desc->imagepatch_addr[0U].dim_x;
        vlib_src1.dim_y = src1_desc->imagepatch_addr[0U].dim_y;
        vlib_src1.stride_y = src1_desc->imagepatch_addr[0U].stride_y;
        vlib_src1.data_type = VXLIB_UINT8;

        vlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
        vlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        vlib_dst.data_type = VXLIB_UINT8;

        /* Get the correct offset of the images from teh valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src0_desc->valid_roi;

        src0_addr = (uint8_t *)((uint32_t)src0_desc->mem_ptr[0U].target_ptr +
            bitwiseComputePatchOffset(rect.start_x, rect.start_y,
            &src0_desc->imagepatch_addr[0U]));
        src1_addr = (uint8_t *)((uint32_t)src1_desc->mem_ptr[0U].target_ptr +
            bitwiseComputePatchOffset(rect.start_x, rect.start_y,
            &src1_desc->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uint32_t)dst_desc->mem_ptr[0U].target_ptr +
            bitwiseComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        if (bitwise_fxn)
        {
            status = bitwise_fxn(src0_addr, &vlib_src0, src1_addr, &vlib_src1,
                dst_addr, &vlib_dst);

            if (VXLIB_SUCCESS == status)
            {
                tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
                    dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
                    VX_WRITE_ONLY);
            }
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

static vx_status tivxKernelBitwiseNotProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width, height, x, y;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src_addr, *dst_addr;

    if ((num_params != 2U) || (NULL == obj_desc[0U]) || (NULL == obj_desc[1U]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_OUT_IMG_IDX];

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
                vx_uint8 *src = vxFormatImagePatchAddress2d(
                    src_addr, x, y, &src_desc->imagepatch_addr[0]);
                vx_uint8 *dst = vxFormatImagePatchAddress2d(
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

static vx_status tivxKernelBitwiseAndProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return tivxKernelBitwiseProcess(kernel, obj_desc, num_params,
        VXLIB_and_i8u_i8u_o8u);
}

static vx_status tivxKernelBitwiseOrProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return tivxKernelBitwiseProcess(kernel, obj_desc, num_params,
        VXLIB_or_i8u_i8u_o8u);
}

static vx_status tivxKernelBitwiseXorProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params)
{
    return tivxKernelBitwiseProcess(kernel, obj_desc, num_params,
        VXLIB_xor_i8u_i8u_o8u);
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
    vx_status status = VX_SUCCESS;
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

        if (VX_SUCCESS == status)
        {
            vx_bitwise_not_target_kernel = tivxAddTargetKernel(
                VX_KERNEL_NOT,
                target_name,
                tivxKernelBitwiseNotProcess,
                tivxKernelBitwiseCreate,
                tivxKernelBitwiseDelete,
                tivxKernelBitwiseControl);
            if (NULL == vx_bitwise_not_target_kernel)
            {
                status = VX_FAILURE;
            }
        }

        if (VX_SUCCESS == status)
        {
            vx_bitwise_and_target_kernel = tivxAddTargetKernel(
                VX_KERNEL_AND,
                target_name,
                tivxKernelBitwiseAndProcess,
                tivxKernelBitwiseCreate,
                tivxKernelBitwiseDelete,
                tivxKernelBitwiseControl);
            if (NULL == vx_bitwise_and_target_kernel)
            {
                status = VX_FAILURE;
            }
        }
        if (VX_SUCCESS == status)
        {
            vx_bitwise_or_target_kernel = tivxAddTargetKernel(
                VX_KERNEL_OR,
                target_name,
                tivxKernelBitwiseOrProcess,
                tivxKernelBitwiseCreate,
                tivxKernelBitwiseDelete,
                tivxKernelBitwiseControl);
            if (NULL == vx_bitwise_or_target_kernel)
            {
                status = VX_FAILURE;
            }
        }
        if (VX_SUCCESS == status)
        {
            vx_bitwise_xor_target_kernel = tivxAddTargetKernel(
                VX_KERNEL_XOR,
                target_name,
                tivxKernelBitwiseXorProcess,
                tivxKernelBitwiseCreate,
                tivxKernelBitwiseDelete,
                tivxKernelBitwiseControl);
            if (NULL == vx_bitwise_xor_target_kernel)
            {
                status = VX_FAILURE;
            }
        }
    }
}


void tivxRemoveTargetKernelBitwise()
{
    if (NULL != vx_bitwise_not_target_kernel)
    {
        tivxRemoveTargetKernel(vx_bitwise_not_target_kernel);
        vx_bitwise_not_target_kernel = NULL;
    }
    if (NULL != vx_bitwise_and_target_kernel)
    {
        tivxRemoveTargetKernel(vx_bitwise_and_target_kernel);
        vx_bitwise_and_target_kernel = NULL;
    }
    if (NULL != vx_bitwise_or_target_kernel)
    {
        tivxRemoveTargetKernel(vx_bitwise_or_target_kernel);
        vx_bitwise_or_target_kernel = NULL;
    }
    if (NULL != vx_bitwise_xor_target_kernel)
    {
        tivxRemoveTargetKernel(vx_bitwise_xor_target_kernel);
        vx_bitwise_xor_target_kernel = NULL;
    }
}
