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
#include <tivx_kernel_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

#define SCRATCH_BUFFER_SIZE 1024

static tivx_target_kernel vx_histogram_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_distribution_t *dst;
    uint8_t *src_addr;
    VXLIB_bufParams2D_t vxlib_src;
    vx_rectangle_t rect;
    void *scratch;
    uint32_t scratch_size;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_HISTOGRAM_OUT_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr.shared_ptr, dst->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr.target_ptr, dst->mem_size,
            dst->mem_ptr.mem_type, VX_WRITE_ONLY);

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

        status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

        if (VXLIB_SUCCESS == status)
        {
            memset(dst->mem_ptr.target_ptr, 0, dst->mem_size);

            status = VXLIB_histogram_i8u_o32u(src_addr, &vxlib_src,
                    dst->mem_ptr.target_ptr, (uint32_t*)scratch, dst->offset, dst->range, dst->num_bins, 1);
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
        tivxMemBufferUnmap(dst->mem_ptr.target_ptr, dst->mem_size,
            dst->mem_ptr.mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
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

        temp_ptr = tivxMemAlloc(SCRATCH_BUFFER_SIZE *
            sizeof(uint32_t));

        if (NULL == temp_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            memset(temp_ptr, 0, SCRATCH_BUFFER_SIZE *
                sizeof(uint32_t));
            tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                             SCRATCH_BUFFER_SIZE * sizeof(uint32_t));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;

    if (num_params != TIVX_KERNEL_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HISTOGRAM_MAX_PARAMS; i ++)
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

        status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

        if (VXLIB_SUCCESS != status)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            tivxMemFree(temp_ptr, temp_ptr_size);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHistogramControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelHistogram(void)
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

        vx_histogram_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HISTOGRAM,
            target_name,
            tivxKernelHistogramProcess,
            tivxKernelHistogramCreate,
            tivxKernelHistogramDelete,
            tivxKernelHistogramControl,
            NULL);
    }
}


void tivxRemoveTargetKernelHistogram(void)
{
    tivxRemoveTargetKernel(vx_histogram_target_kernel);
}
