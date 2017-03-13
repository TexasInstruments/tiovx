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
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_sobel_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelSobelProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst0, *dst1;
    uint8_t *src_addr;
    int16_t *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_SOBEL_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Check for NULL */
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX];
        dst0 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
        dst1 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        if (NULL != dst0)
        {
            dst0->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                dst0->mem_ptr[0].shared_ptr, dst0->mem_ptr[0].mem_type);

            tivxMemBufferMap(dst0->mem_ptr[0].target_ptr, dst0->mem_size[0],
                dst0->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            /* TODO: Do we require to move pointer even for destination image */
            dst_addr = (int16_t *)((uintptr_t)dst0->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x + 1U, rect.start_y + 1U,
                &dst0->imagepatch_addr[0]));

            vxlib_dst.dim_x = dst0->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst0->imagepatch_addr[0].dim_y - 2U;
            vxlib_dst.stride_y = dst0->imagepatch_addr[0].stride_y;
            vxlib_dst.data_type = VXLIB_INT16;

            status = VXLIB_sobelX_3x3_i8u_o16s(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst);

            if (status != VXLIB_SUCCESS)
            {
                status = VX_FAILURE;
            }

            tivxMemBufferUnmap(dst0->mem_ptr[0].target_ptr, dst0->mem_size[0],
                dst0->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        }

        if ((VX_SUCCESS == status) && (NULL != dst1))
        {
            dst1->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                dst1->mem_ptr[0].shared_ptr, dst1->mem_ptr[0].mem_type);

            tivxMemBufferMap(dst1->mem_ptr[0].target_ptr, dst1->mem_size[0],
                dst1->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            /* TODO: Do we require to move pointer even for destination image */
            dst_addr = (int16_t *)((uintptr_t)dst1->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x + 1U, rect.start_y + 1U,
                &dst1->imagepatch_addr[0]));

            vxlib_dst.dim_x = dst1->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst1->imagepatch_addr[0].dim_y - 2U;
            vxlib_dst.stride_y = dst1->imagepatch_addr[0].stride_y;
            vxlib_dst.data_type = VXLIB_INT16;

            status = VXLIB_sobelY_3x3_i8u_o16s(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst);

            if (status != VXLIB_SUCCESS)
            {
                status = VX_FAILURE;
            }

            tivxMemBufferUnmap(dst1->mem_ptr[0].target_ptr, dst1->mem_size[0],
                dst1->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSobelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSobelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSobelControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelSobel3x3(void)
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

        vx_sobel_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_SOBEL_3x3,
            target_name,
            tivxKernelSobelProcess,
            tivxKernelSobelCreate,
            tivxKernelSobelDelete,
            tivxKernelSobelControl,
            NULL);
    }
}


void tivxRemoveTargetKernelSobel3x3(void)
{
    tivxRemoveTargetKernel(vx_sobel_target_kernel);
}
