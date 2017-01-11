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
#include <tivx_kernel_phase.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_phase_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelPhaseProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    vx_uint8 *dst_addr;
    vx_int16 *src0_addr, *src1_addr;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_PHASE_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_PHASE_MAX_PARAMS; i ++)
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
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_PHASE_IN0_IMG_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_PHASE_IN1_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_PHASE_OUT_IMG_IDX];

        src0->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0->mem_ptr[0].shared_ptr, src0->mem_ptr[0].mem_type);
        src1->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1->mem_ptr[0].shared_ptr, src1->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(src1->mem_ptr[0].target_ptr, src1->mem_size[0],
            src1->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src0->valid_roi;

        src0_addr = (int16_t *)((uintptr_t)src0->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src0->imagepatch_addr[0U]));
        src1_addr = (int16_t *)((uintptr_t)src1->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src1->imagepatch_addr[0U]));

        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        vxlib_src0.dim_x = src0->imagepatch_addr[0].dim_x;
        vxlib_src0.dim_y = src0->imagepatch_addr[0].dim_y;
        vxlib_src0.stride_y = src0->imagepatch_addr[0].stride_y;
        vxlib_src0.data_type = VXLIB_INT16;

        vxlib_src1.dim_x = src1->imagepatch_addr[0].dim_x;
        vxlib_src1.dim_y = src1->imagepatch_addr[0].dim_y;
        vxlib_src1.stride_y = src1->imagepatch_addr[0].stride_y;
        vxlib_src1.data_type = VXLIB_INT16;

        vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        status = VXLIB_phase_i16s_i16s_o8u(src0_addr, &vxlib_src0,
            src1_addr, &vxlib_src1, dst_addr, &vxlib_dst);

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(src1->mem_ptr[0].target_ptr, src1->mem_size[0],
            src1->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelPhaseCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelPhaseDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelPhaseControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelPhase()
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

        vx_phase_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_PHASE,
            target_name,
            tivxKernelPhaseProcess,
            tivxKernelPhaseCreate,
            tivxKernelPhaseDelete,
            tivxKernelPhaseControl,
            NULL);
    }
}


void tivxRemoveTargetKernelPhase()
{
    tivxRemoveTargetKernel(vx_phase_target_kernel);
}
