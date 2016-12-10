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
#include <tivx_kernel_warp_affine.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <stdio.h>

static tivx_target_kernel vx_warp_affine_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelWarpAffineProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_matrix_t *mat;
    tivx_obj_desc_scalar_t *sc;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    float *mat_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    vx_rectangle_t rect;
    vx_border_t border;

    if (num_params != TIVX_KERNEL_WARP_AFFINE_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_WARP_AFFINE_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_WARP_AFFINE_IN0_IMG_IDX];
        mat = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_WARP_AFFINE_IN0_MAT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_WARP_AFFINE_OUT_IMG_IDX];

        sc = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_WARP_AFFINE_IN0_SC_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        mat->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            mat->mem_ptr.shared_ptr, mat->mem_ptr.mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(mat->mem_ptr.target_ptr, mat->mem_size,
            mat->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        mat_addr = (float *)((uint32_t)mat->mem_ptr.target_ptr);

        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uint32_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        tivxGetTargetKernelInstanceBorderMode(kernel, &border);

        /* If interpolation is nearest neighbor */
        if (VX_INTERPOLATION_NEAREST_NEIGHBOR == sc->data.enm)
        {
            status = VXLIB_warpAffineNearest_bc_i8u_c32f_o8u(
                    src_addr, &vxlib_src,
                    dst_addr, &vxlib_dst,
                    mat_addr, border.constant_value.U8,
                    0, 0, 0, 0);
        }
        /* If interpolation is Bilinear */
        else
        {
            status = VXLIB_warpAffineBilinear_bc_i8u_c32f_o8u(
                    src_addr, &vxlib_src,
                    dst_addr, &vxlib_dst,
                    mat_addr, border.constant_value.U8,
                    0, 0, 0, 0);
        }
        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(mat->mem_ptr.target_ptr, mat->mem_size,
            mat->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelWarpAffineCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelWarpAffineDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelWarpAffineControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelWarpAffine()
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

        vx_warp_affine_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_WARP_AFFINE,
            target_name,
            tivxKernelWarpAffineProcess,
            tivxKernelWarpAffineCreate,
            tivxKernelWarpAffineDelete,
            tivxKernelWarpAffineControl,
            NULL);
    }
}


void tivxRemoveTargetKernelWarpAffine()
{
    tivxRemoveTargetKernel(vx_warp_affine_target_kernel);
}
