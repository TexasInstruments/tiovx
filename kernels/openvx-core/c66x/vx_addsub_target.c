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
#include <tivx_kernel_addsub.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_add_target_kernel = NULL, vx_sub_target_kernel = NULL;

static vx_status tivxKernelAddSub(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    vx_enum kern_type)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    tivx_obj_desc_scalar_t *sc_desc;
    uint32_t i;
    void *src0_addr, *src1_addr, *dst_addr;
    vx_rectangle_t rect;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
    uint16_t overflow_policy;

    if (num_params != TIVX_KERNEL_ADDSUB_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_ADDSUB_MAX_PARAMS; i ++)
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
        src0_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_ADDSUB_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_ADDSUB_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)
            obj_desc[TIVX_KERNEL_ADDSUB_OUT_IMG_IDX];
        sc_desc = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_ADDSUB_IN_SCALAR_IDX];

        src0_desc->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0U].shared_ptr, src0_desc->mem_ptr[0U].mem_type);
        src1_desc->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0U].shared_ptr, src1_desc->mem_ptr[0U].mem_type);

        dst_desc->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0U].shared_ptr, dst_desc->mem_ptr[0U].mem_type);

        tivxMemBufferMap(src0_desc->mem_ptr[0U].target_ptr, src0_desc->mem_size[0],
            src0_desc->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0U].target_ptr, src1_desc->mem_size[0],
            src1_desc->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

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

        vxlib_src0.dim_x = src0_desc->imagepatch_addr[0].dim_x;
        vxlib_src0.dim_y = src0_desc->imagepatch_addr[0].dim_y;
        vxlib_src0.stride_y = src0_desc->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == src0_desc->format)
        {
            vxlib_src0.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_src0.data_type = VXLIB_INT16;
        }

        vxlib_src1.dim_x = src1_desc->imagepatch_addr[0].dim_x;
        vxlib_src1.dim_y = src1_desc->imagepatch_addr[0].dim_y;
        vxlib_src1.stride_y = src1_desc->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == src1_desc->format)
        {
            vxlib_src1.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_src1.data_type = VXLIB_INT16;
        }

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == dst_desc->format)
        {
            vxlib_dst.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_dst.data_type = VXLIB_INT16;
        }

        if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
        {
            overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
        }
        else
        {
            overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
        }

        if (VX_KERNEL_ADD == kern_type)
        {
            /* If output is in U8 format, both the input must be in
               U8 format */
            if (VXLIB_UINT8 == vxlib_dst.data_type)
            {
                status = VXLIB_add_i8u_i8u_o8u((uint8_t *)src0_addr,
                    &vxlib_src0, (uint8_t *)src1_addr, &vxlib_src1,
                    (uint8_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            /* Now if the both inputs are U8, output will be in S16 format */
            else if ((VXLIB_UINT8 == vxlib_src1.data_type) &&
                     (VXLIB_UINT8 == vxlib_src0.data_type))
            {
                status = VXLIB_add_i8u_i8u_o16s((uint8_t *)src0_addr,
                    &vxlib_src0, (uint8_t *)src1_addr, &vxlib_src1,
                    (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            /* If both the input are in S16 format, output will be in
               S16 format */
            else if ((VXLIB_INT16 == vxlib_src1.data_type) &&
                     (VXLIB_INT16 == vxlib_src0.data_type))
            {
                status = VXLIB_add_i16s_i16s_o16s((int16_t *)src0_addr,
                    &vxlib_src0, (int16_t *)src1_addr, &vxlib_src1,
                    (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            else /* One input is in S16 format and other is in U8 format */
            {

                if (VXLIB_UINT8 == vxlib_src0.data_type)
                {
                    status = VXLIB_add_i8u_i16s_o16s((uint8_t *)src0_addr,
                        &vxlib_src0, (int16_t *)src1_addr, &vxlib_src1,
                        (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
                }
                else
                {
                    status = VXLIB_add_i8u_i16s_o16s((uint8_t *)src1_addr,
                        &vxlib_src1, (int16_t *)src0_addr, &vxlib_src0,
                        (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
                }
            }
        }
        else /* Kernel Type is subtraction */
        {
            /* If output is in U8 format, both the input must be in
               U8 format */
            if (VXLIB_UINT8 == vxlib_dst.data_type)
            {
                status = VXLIB_subtract_i8u_i8u_o8u((uint8_t *)src0_addr,
                    &vxlib_src0, (uint8_t *)src1_addr, &vxlib_src1,
                    (uint8_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            /* Now if the both inputs are U8, output will be in S16 format */
            else if ((VXLIB_UINT8 == vxlib_src1.data_type) &&
                     (VXLIB_UINT8 == vxlib_src0.data_type))
            {
                status = VXLIB_subtract_i8u_i8u_o16s((uint8_t *)src0_addr,
                    &vxlib_src0, (uint8_t *)src1_addr, &vxlib_src1,
                    (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            /* If both the input are in S16 format, output will be in
               S16 format */
            else if ((VXLIB_INT16 == vxlib_src1.data_type) &&
                     (VXLIB_INT16 == vxlib_src0.data_type))
            {
                status = VXLIB_subtract_i16s_i16s_o16s((int16_t *)src0_addr,
                    &vxlib_src0, (int16_t *)src1_addr, &vxlib_src1,
                    (int16_t *)dst_addr, &vxlib_dst, overflow_policy);
            }
            else /* One input is in S16 format and other is in U8 format */
            {
                if (VXLIB_UINT8 != vxlib_src0.data_type)
                {
                    status = VXLIB_subtract_i8u_i16s_o16s((uint8_t *)src1_addr,
                        &vxlib_src1, (int16_t *)src0_addr, &vxlib_src0,
                        (int16_t *)dst_addr, &vxlib_dst, overflow_policy,
                        1);
                }
                else
                {
                    status = VXLIB_subtract_i8u_i16s_o16s((uint8_t *)src0_addr,
                        &vxlib_src0, (int16_t *)src1_addr, &vxlib_src1,
                        (int16_t *)dst_addr, &vxlib_dst, overflow_policy,
                        0);
                }
            }
        }

        tivxMemBufferUnmap(dst_desc->mem_ptr[0U].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0U].mem_type,
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

    status = tivxKernelAddSub(kernel, obj_desc, num_params, VX_KERNEL_ADD);

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

    status = tivxKernelAddSub(kernel, obj_desc, num_params, VX_KERNEL_SUBTRACT);

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

