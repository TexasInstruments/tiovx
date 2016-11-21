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
#include <tivx_kernel_threshold.h>
#include <TI/tivx_target_kernel.h>

static tivx_target_kernel vx_threshold_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelThresholdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i, j, w, h;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    int32_t value, upper, lower;
    vx_uint8 *src_ptr, *dst_ptr;
    vx_enum type;

    if ((num_params != 3U) || (NULL == obj_desc[0]) || (NULL == obj_desc[1])
        || (NULL == obj_desc[2]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRLD_IN_IMG_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[TIVX_KERNEL_THRLD_IN_THR_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_THRLD_OUT_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        type = thr->type;
        value = thr->value;
        upper = thr->upper;
        lower = thr->lower;

        w = src->imagepatch_addr[0].dim_x;
        h = src->imagepatch_addr[0].dim_y;
        src_ptr = (vx_uint8 *)src->mem_ptr[0].target_ptr;
        dst_ptr = (vx_uint8 *)dst->mem_ptr[0].target_ptr;

        for (i = 0; i < h; i ++)
        {
            for (j = 0; j < w; j ++)
            {
                if (VX_THRESHOLD_TYPE_BINARY == type)
                {
                    if (*src_ptr > value)
                    {
                        *dst_ptr = 0xFF;
                    }
                    else
                    {
                        *dst_ptr = 0;
                    }
                }
                else
                {
                    if ((*src_ptr > upper) || (*src_ptr < lower))
                    {
                        *dst_ptr = 0;
                    }
                    else
                    {
                        *dst_ptr = 0xFF;
                    }
                }
                src_ptr ++;
                dst_ptr ++;
            }
        }

        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelThresholdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelThresholdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelThresholdControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelThreshold()
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

        vx_threshold_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_THRESHOLD,
            target_name,
            tivxKernelThresholdProcess,
            tivxKernelThresholdCreate,
            tivxKernelThresholdDelete,
            tivxKernelThresholdControl,
            NULL);
    }
}


void tivxRemoveTargetKernelThreshold()
{
    tivxRemoveTargetKernel(vx_threshold_target_kernel);
}
