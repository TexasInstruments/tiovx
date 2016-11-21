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
#include <tivx_kernel_addsub.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_add_target_kernel = NULL, vx_sub_target_kernel = NULL;

typedef vx_int32 (arithmeticOp)(vx_int32, vx_int32);

static vx_int32 vx_add_op(vx_int32 a, vx_int32 b)
{
    return (a + b);
}

static vx_int32 vx_sub_op(vx_int32 a, vx_int32 b)
{
    return (a - b);
}

static vx_status tivxKernelAddSub(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    arithmeticOp op)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    tivx_obj_desc_scalar_t *sc_desc;
    uint32_t w, h, i, j;
    vx_enum policy;
    void *src0_addr, *src1_addr, *dst_addr;
    int32_t src0_val, src1_val, dst_val;
    int32_t result;

    if ((num_params != 4U) || (NULL == obj_desc[0]) || (NULL == obj_desc[1]) ||
        (NULL == obj_desc[2]) || (NULL == obj_desc[3]))
    {
        status = VX_FAILURE;
    }
    else
    {
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_OUT_IMG_IDX];
        sc_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_ADDSUB_IN_SCALAR_IDX];

        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);

        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr, src0_desc->mem_size[0],
            src0_desc->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr, src1_desc->mem_size[0],
            src1_desc->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        w = src0_desc->imagepatch_addr[0].dim_x;
        h = src0_desc->imagepatch_addr[0].dim_y;

        policy = sc_desc->data.enm;

        for (i = 0; i < h; i ++)
        {
            for (j = 0; j < w; j ++)
            {
                src0_addr = ownFormatImagePatchAddress2d(
                    src0_desc->mem_ptr[0].target_ptr,
                    j, i, &src0_desc->imagepatch_addr[0]);
                src1_addr = ownFormatImagePatchAddress2d(
                    src1_desc->mem_ptr[0].target_ptr,
                    j, i, &src1_desc->imagepatch_addr[0]);
                dst_addr = ownFormatImagePatchAddress2d(
                    dst_desc->mem_ptr[0].target_ptr,
                    j, i, &dst_desc->imagepatch_addr[0]);

                if (VX_DF_IMAGE_U8 == src0_desc->format)
                {
                    src0_val = *(vx_uint8 *)src0_addr;
                }
                else
                {
                    src0_val = *(vx_int16 *)src0_addr;
                }
                if (VX_DF_IMAGE_U8 == src1_desc->format)
                {
                    src1_val = *(vx_uint8 *)src1_addr;
                }
                else
                {
                    src1_val = *(vx_int16 *)src1_addr;
                }

                result = op(src0_val, src1_val);

                if (VX_CONVERT_POLICY_SATURATE == policy)
                {
                    if (VX_DF_IMAGE_U8 == dst_desc->format)
                    {
                        if (result > UINT8_MAX)
                        {
                            dst_val = UINT8_MAX;
                        }
                        else if (result < 0)
                        {
                            dst_val = 0;
                        }
                        else
                        {
                            dst_val = result;
                        }
                    }
                    else
                    {
                        if (result > INT16_MAX)
                        {
                            dst_val = INT16_MAX;
                        }
                        else if (result < INT16_MIN)
                        {
                            dst_val = INT16_MIN;
                        }
                        else
                        {
                            dst_val = result;
                        }
                    }
                }
                else
                {
                    if (VX_DF_IMAGE_U8 == dst_desc->format)
                    {
                        dst_val = (uint8_t)result;
                    }
                    else
                    {
                        dst_val = (int16_t)result;
                    }
                }

                if (VX_DF_IMAGE_U8 == dst_desc->format)
                {
                    *(uint8_t *)dst_addr = dst_val;
                }
                else
                {
                    *(int16_t *)dst_addr = dst_val;
                }
            }
        }

        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAddCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAddDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAddControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelAddProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelAddSub(kernel, obj_desc, num_params, vx_add_op);

    return (status);
}

void tivxAddTargetKernelAdd()
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

        vx_add_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_ADD,
            target_name,
            tivxKernelAddProcess,
            tivxKernelAddCreate,
            tivxKernelAddDelete,
            tivxKernelAddControl,
            NULL);
    }
}


void tivxRemoveTargetKernelAdd()
{
    tivxRemoveTargetKernel(vx_add_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelSubCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSubDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSubControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSubProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status;

    status = tivxKernelAddSub(kernel, obj_desc, num_params, vx_sub_op);

    return (status);
}

void tivxAddTargetKernelSub()
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

        vx_sub_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_SUBTRACT,
            target_name,
            tivxKernelSubProcess,
            tivxKernelSubCreate,
            tivxKernelSubDelete,
            tivxKernelSubControl,
            NULL);
    }
}


void tivxRemoveTargetKernelSub()
{
    tivxRemoveTargetKernel(vx_sub_target_kernel);
}

