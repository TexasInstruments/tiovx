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
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_remap.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_remap_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelRemapProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_scalar_t *sc;
    vx_uint8 *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst, vxlib_remap;
    vx_rectangle_t rect;
    vx_border_t border;
    tivx_obj_desc_remap_t *remap;

    if (num_params != TIVX_KERNEL_REMAP_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_REMAP_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_REMAP_IN_IMG_IDX];
        sc = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_REMAP_IN_POLICY_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_REMAP_OUT_IMG_IDX];
        remap = (tivx_obj_desc_remap_t *)obj_desc[TIVX_KERNEL_REMAP_IN_TBL_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);
        remap->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            remap->mem_ptr.shared_ptr, remap->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        tivxMemBufferMap(remap->mem_ptr.target_ptr, remap->mem_size,
            remap->mem_ptr.mem_type, VX_READ_ONLY);

        /* Get the correct offset of the images from teh valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
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

        vxlib_remap.dim_x = dst->imagepatch_addr[0].dim_x * 2U;
        vxlib_remap.dim_y = dst->imagepatch_addr[0].dim_y;
        vxlib_remap.stride_y = dst->imagepatch_addr[0].stride_y * 8U;
        vxlib_remap.data_type = VXLIB_FLOAT32;

        tivxGetTargetKernelInstanceBorderMode(kernel, &border);

        if (VX_INTERPOLATION_BILINEAR == sc->data.enm)
        {
            status = VXLIB_remapBilinear_bc_i8u_i32f_o8u(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst,
                remap->mem_ptr.target_ptr, &vxlib_remap,
                border.constant_value.U8);
        }
        else if (VX_INTERPOLATION_NEAREST_NEIGHBOR == sc->data.enm)
        {
            status = VXLIB_remapNearest_bc_i8u_i32f_o8u(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst,
                remap->mem_ptr.target_ptr, &vxlib_remap,
                border.constant_value.U8);
        }
        else
        {
            status = VX_FAILURE;
        }

        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        tivxMemBufferUnmap(remap->mem_ptr.target_ptr, remap->mem_size,
            remap->mem_ptr.mem_type, VX_READ_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelRemapCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelRemapDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelRemapControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelRemap(void)
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

        vx_remap_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_REMAP,
            target_name,
            tivxKernelRemapProcess,
            tivxKernelRemapCreate,
            tivxKernelRemapDelete,
            tivxKernelRemapControl,
            NULL);
    }
}


void tivxRemoveTargetKernelRemap(void)
{
    tivxRemoveTargetKernel(vx_remap_target_kernel);
}
