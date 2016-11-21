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
#include <tivx_kernel_absdiff.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_absdiff_target_kernel = NULL;

vx_status tivxAbsDiff(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
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
            TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX];

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
        if (VX_DF_IMAGE_U8 == src0_desc->format)
        {
            vlib_src0.data_type = VXLIB_UINT8;
        }
        else
        {
            vlib_src0.data_type = VXLIB_INT16;
        }

        vlib_src1.dim_x = src1_desc->imagepatch_addr[0U].dim_x;
        vlib_src1.dim_y = src1_desc->imagepatch_addr[0U].dim_y;
        vlib_src1.stride_y = src1_desc->imagepatch_addr[0U].stride_y;
        if (VX_DF_IMAGE_U8 == src1_desc->format)
        {
            vlib_src1.data_type = VXLIB_UINT8;
        }
        else
        {
            vlib_src1.data_type = VXLIB_INT16;
        }

        vlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
        vlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        if (VX_DF_IMAGE_U8 == dst_desc->format)
        {
            vlib_dst.data_type = VXLIB_UINT8;
        }
        else
        {
            vlib_dst.data_type = VXLIB_INT16;
        }

        /* Get the correct offset of the images from teh valid roi parameter,
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

        if (VXLIB_UINT8 == vlib_dst.data_type)
        {
            status = VXLIB_absDiff_i8u_i8u_o8u(src0_addr, &vlib_src0, src1_addr,
                &vlib_src1, dst_addr, &vlib_dst);
        }
        else
        {
            status = VXLIB_absDiff_i16s_i16s_o16s(
                (int16_t *)src0_addr, &vlib_src0, (int16_t *)src1_addr,
                &vlib_src1, (int16_t *)dst_addr, &vlib_dst);
        }

        if (VXLIB_SUCCESS == status)
        {
            tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
                dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxAbsDiffCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxAbsDiffControl(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void tivxAddTargetKernelAbsDiff()
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

        vx_absdiff_target_kernel = tivxAddTargetKernel(
                    VX_KERNEL_ABSDIFF,
                    target_name,
                    tivxAbsDiff,
                    tivxAbsDiffCreate,
                    tivxAbsDiffDelete,
                    tivxAbsDiffControl,
                    NULL);
    }
}

void tivxRemoveTargetKernelAbsDiff()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_absdiff_target_kernel);

    if (VX_SUCCESS == status)
    {
        vx_absdiff_target_kernel = NULL;
    }
}

