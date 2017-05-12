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
#include <tivx_kernel_intgimg.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_target_kernels_utils.h>

static tivx_target_kernel vx_intgimg_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelIntgImgProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr;
    uint32_t *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint32_t *prev_row, size;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_INTGIMG_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        ownInitBufParams(src, NULL, &vxlib_src, &src_addr, 0, 0, 0, 0);
        ownInitBufParams(dst, NULL, &vxlib_dst, (uint8_t **)&dst_addr,
            0, 0, 0, 0);

        tivxGetTargetKernelInstanceContext(kernel, (void **)&prev_row, &size);

        if (NULL != prev_row)
        {
            status = VXLIB_integralImage_i8u_o32u(src_addr, &vxlib_src,
                    dst_addr, &vxlib_dst, prev_row, NULL, 0);
        }
        else
        {
            status = VX_FAILURE;
        }

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelIntgImgCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *dst;
    void *temp_ptr;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        temp_ptr = tivxMemAlloc(dst->imagepatch_addr[0].dim_x *
            sizeof(uint32_t), TIVX_MEM_EXTERNAL);

        if (NULL == temp_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            memset(temp_ptr, 0, dst->imagepatch_addr[0].dim_x *
                sizeof(uint32_t));
            tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                dst->imagepatch_addr[0].dim_x * sizeof(uint32_t));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelIntgImgDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivx_obj_desc_image_t *dst;
    void *temp_ptr;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &size);

        if (NULL == temp_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            tivxMemFree(temp_ptr , (dst->imagepatch_addr[0].dim_x *
                sizeof(uint32_t)), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelIntgImgControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelIntegralImage(void)
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

        vx_intgimg_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_INTEGRAL_IMAGE,
            target_name,
            tivxKernelIntgImgProcess,
            tivxKernelIntgImgCreate,
            tivxKernelIntgImgDelete,
            tivxKernelIntgImgControl,
            NULL);
    }
}


void tivxRemoveTargetKernelIntegralImage(void)
{
    tivxRemoveTargetKernel(vx_intgimg_target_kernel);
}
