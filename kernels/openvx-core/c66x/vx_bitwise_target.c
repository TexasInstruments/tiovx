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
#include <tivx_kernel_bitwise.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

typedef VXLIB_STATUS (*VxLib_Bitwise_Fxn)(
    const uint8_t *src0, const VXLIB_bufParams2D_t *src0_prms,
    const uint8_t *src1, const VXLIB_bufParams2D_t *src1_prms,
    uint8_t *dst, const VXLIB_bufParams2D_t *dst_prms);

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Pointer to the kernel Registered */
    tivx_target_kernel      target_kernel;

    /*! \brief Pointer to filter function */
    VxLib_Bitwise_Fxn       vxlib_process;
} tivxBitwiseKernelInfo;

tivxBitwiseKernelInfo gTivxBitwiseKernelInfo[] =
{
    {
        VX_KERNEL_OR,
        NULL,
        VXLIB_or_i8u_i8u_o8u
    },
    {
        VX_KERNEL_XOR,
        NULL,
        VXLIB_xor_i8u_i8u_o8u
    },
    {
        VX_KERNEL_AND,
        NULL,
        VXLIB_and_i8u_i8u_o8u
    },
    {
        VX_KERNEL_NOT,
        NULL,
        NULL
    }
};

vx_status VX_CALLBACK tivxKernelBitwiseProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
    tivxBitwiseKernelInfo *kern_info;

    if ((num_params != 3U) || (NULL == obj_desc[0U]) ||
        (NULL == obj_desc[1U]) || (NULL == obj_desc[2U]) ||
        (NULL == kernel) || (NULL == priv_arg))
    {
        status = VX_FAILURE;
    }
    else
    {
        kern_info = (tivxBitwiseKernelInfo *)priv_arg;

        /* Get the Src and Dst descriptors */
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src0_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_type);
        src1_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        /* Map all three buffers, which invalidates the cache */
        tivxMemBufferMap(src0_desc->mem_ptr[0].target_ptr,
            src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(src1_desc->mem_ptr[0].target_ptr,
            src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src0.dim_x = src0_desc->imagepatch_addr[0U].dim_x;
        vxlib_src0.dim_y = src0_desc->imagepatch_addr[0U].dim_y;
        vxlib_src0.stride_y = src0_desc->imagepatch_addr[0U].stride_y;
        vxlib_src0.data_type = VXLIB_UINT8;

        vxlib_src1.dim_x = src1_desc->imagepatch_addr[0U].dim_x;
        vxlib_src1.dim_y = src1_desc->imagepatch_addr[0U].dim_y;
        vxlib_src1.stride_y = src1_desc->imagepatch_addr[0U].stride_y;
        vxlib_src1.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src0_desc->valid_roi;

        src0_addr = (uint8_t *)((uintptr_t)src0_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src0_desc->imagepatch_addr[0U]));
        src1_addr = (uint8_t *)((uintptr_t)src1_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src1_desc->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        if (NULL != kern_info->vxlib_process)
        {
            status = kern_info->vxlib_process(
                src0_addr, &vxlib_src0, src1_addr, &vxlib_src1,
                dst_addr, &vxlib_dst);

            if (VXLIB_SUCCESS != status)
            {
                status = VX_FAILURE;
            }
        }
        else
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src0_desc->mem_ptr[0].target_ptr,
            src0_desc->mem_size[0], src0_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src1_desc->mem_ptr[0].target_ptr,
            src1_desc->mem_size[0], src1_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
    }

    return (status);
}

vx_status VX_CALLBACK tivxKernelBitwiseNotProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    vx_rectangle_t rect;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t *src_addr, *dst_addr;

    if ((num_params != 2U) || (NULL == obj_desc[0U]) ||
        (NULL == obj_desc[1U]) || (NULL == kernel) ||
        (NULL == priv_arg))
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src_desc->mem_ptr[0].target_ptr,
            src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Initialize vxLib Parameters with the input/output frame parameters */
        vxlib_src.dim_x = src_desc->imagepatch_addr[0U].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0U].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0U].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0]));

        status = VXLIB_not_i8u_o8u(src_addr, &vxlib_src, dst_addr, &vxlib_dst);

        if (VXLIB_SUCCESS == status)
        {
            tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
                dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
                VX_WRITE_ONLY);
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBitwiseCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBitwise(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    vx_uint32 i;

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

        for (i = 0; i < (sizeof(gTivxBitwiseKernelInfo)/
                sizeof(tivxBitwiseKernelInfo)); i ++)
        {
            if (VX_KERNEL_NOT == gTivxBitwiseKernelInfo[i].kernel_id)
            {
                gTivxBitwiseKernelInfo[i].target_kernel = tivxAddTargetKernel(
                    gTivxBitwiseKernelInfo[i].kernel_id,
                    target_name,
                    tivxKernelBitwiseNotProcess,
                    tivxKernelBitwiseCreate,
                    tivxKernelBitwiseDelete,
                    tivxKernelBitwiseControl,
                    &gTivxBitwiseKernelInfo[i]);
            }
            else
            {
                gTivxBitwiseKernelInfo[i].target_kernel = tivxAddTargetKernel(
                    gTivxBitwiseKernelInfo[i].kernel_id,
                    target_name,
                    tivxKernelBitwiseProcess,
                    tivxKernelBitwiseCreate,
                    tivxKernelBitwiseDelete,
                    tivxKernelBitwiseControl,
                    &gTivxBitwiseKernelInfo[i]);
            }

            if (NULL == gTivxBitwiseKernelInfo[i].target_kernel)
            {
                break;
            }
        }
    }
}


void tivxRemoveTargetKernelBitwise(void)
{
    vx_status status;
    vx_uint32 i;

    for (i = 0; i < (sizeof(gTivxBitwiseKernelInfo)/
            sizeof(tivxBitwiseKernelInfo)); i ++)
    {
        if (gTivxBitwiseKernelInfo[i].target_kernel)
        {
            status = tivxRemoveTargetKernel(gTivxBitwiseKernelInfo[i].target_kernel);

            if (VX_SUCCESS == status)
            {
                gTivxBitwiseKernelInfo[i].target_kernel = NULL;
            }
        }
    }
}
