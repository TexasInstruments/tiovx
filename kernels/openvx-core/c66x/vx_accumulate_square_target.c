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
#include <tivx_kernel_accumulate_square.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_accumulate_square_target_kernel = NULL;

/* Work on this section */
static vx_status tivxKernelAccumulateSquare(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    tivx_obj_desc_scalar_t *sc_desc;
    uint32_t i;
    void *src_addr, *dst_addr;
    vx_rectangle_t rect;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;

    if (num_params != TIVX_KERNEL_ACCUMULATE_SQUARE_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_ACCUMULATE_SQUARE_MAX_PARAMS; i ++)
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
            obj_desc[TIVX_KERNEL_ACCUMULATE_SQUARE_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_ACCUMULATE_SQUARE_OUT_IMG_IDX];
        sc_desc = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_ACCUMULATE_SQUARE_IN_SCALAR_IDX];

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

        src_addr = (uint8_t *)((uintptr_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));

        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        vxlib_src.dim_x = src_desc->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_INT16;

        status = VXLIB_accumulateSquareImage_i8u_io16s((uint8_t *)src_addr,
                    &vxlib_src, (int16_t *)dst_addr, &vxlib_dst, sc_desc->data.u32);

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

static vx_status VX_CALLBACK tivxKernelAccumulateSquareCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAccumulateSquareDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAccumulateSquareControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAccumulateSquareProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelAccumulateSquare(kernel, obj_desc, num_params, VX_KERNEL_ACCUMULATE_SQUARE);

    return (status);
}

void tivxAddTargetKernelAccumulateSquare()
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

        vx_accumulate_square_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_ACCUMULATE_SQUARE,
            target_name,
            tivxKernelAccumulateSquareProcess,
            tivxKernelAccumulateSquareCreate,
            tivxKernelAccumulateSquareDelete,
            tivxKernelAccumulateSquareControl,
            NULL);
    }
}


void tivxRemoveTargetKernelAccumulateSquare()
{
    tivxRemoveTargetKernel(vx_accumulate_square_target_kernel);
}

