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
#include <tivx_kernel_filter_3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

typedef VXLIB_STATUS (*VxLib_Filt3x3_Fxn)(const uint8_t *,
    const VXLIB_bufParams2D_t *, uint8_t *, const VXLIB_bufParams2D_t *);

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Pointer to the kernel Registered */
    tivx_target_kernel      target_kernel;

    /*! \brief Pointer to filter function */
    VxLib_Filt3x3_Fxn       filter_func;
} tivxFilter3x3KernelInfo;

tivxFilter3x3KernelInfo gTivxFilt3x3KernelInfo[] =
{
    {
        VX_KERNEL_ERODE_3x3,
        NULL,
        VXLIB_erode_3x3_i8u_o8u
    },
    {
        VX_KERNEL_BOX_3x3,
        NULL,
        VXLIB_box_3x3_i8u_o8u
    },
    {
        VX_KERNEL_DILATE_3x3,
        NULL,
        VXLIB_dilate_3x3_i8u_o8u
    },
    {
        VX_KERNEL_GAUSSIAN_3x3,
        NULL,
        VXLIB_gaussian_3x3_i8u_o8u
    },
    {
        VX_KERNEL_MEDIAN_3x3,
        NULL,
        VXLIB_median_3x3_i8u_o8u
    },
};

vx_status tivxProcess3x3Filter(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    vx_rectangle_t rect;
    uint8_t *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    tivxFilter3x3KernelInfo *kern_info;

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
            TIVX_KERNEL_FILT3x3_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_FILT3x3_OUT_IMG_IDX];

        if (NULL == src_desc || NULL == dst_desc)
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        kern_info = (tivxFilter3x3KernelInfo *)priv_arg;

        /* Get the target pointer from the shared pointer for all
           three buffers */
        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        if ((NULL == src_desc->mem_ptr[0].target_ptr) ||
            (NULL == dst_desc->mem_ptr[0].target_ptr))
        {
            status = VX_ERROR_INVALID_REFERENCE;
        }

        if ((src_desc->imagepatch_addr[0U].stride_y <
                src_desc->imagepatch_addr[0U].dim_x) ||
            (dst_desc->imagepatch_addr[0U].stride_y <
                dst_desc->imagepatch_addr[0U].dim_x))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
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

        /* All 3x3 filter reduces the output size */
        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0U].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0U].dim_y - 2U;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0U].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uint32_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));

        /* TODO: Do we require to move pointer even for destination image */
        /* Need to move destination start pointer by 1 line and 1 pixel */
        dst_addr = (uint8_t *)((uint32_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x + 1, rect.start_y + 1,
            &dst_desc->imagepatch_addr[0]));

        if (kern_info->filter_func)
        {
            status = kern_info->filter_func(src_addr, &vxlib_src, dst_addr,
                &vxlib_dst);
        }
        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src_desc->mem_ptr[0].target_ptr,
            src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
    }

    return (status);
}

vx_status tivxFilter3x3Create(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxFilter3x3Delete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status tivxFilter3x3Control(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelErode3x3()
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

        for (i = 0; i < sizeof(gTivxFilt3x3KernelInfo)/
                sizeof(tivxFilter3x3KernelInfo); i ++)
        {
            gTivxFilt3x3KernelInfo[i].target_kernel = tivxAddTargetKernel(
                        gTivxFilt3x3KernelInfo[i].kernel_id,
                        target_name,
                        tivxProcess3x3Filter,
                        tivxFilter3x3Create,
                        tivxFilter3x3Delete,
                        tivxFilter3x3Control,
                        (void *)&gTivxFilt3x3KernelInfo[i]);
            if (NULL == gTivxFilt3x3KernelInfo[i].target_kernel)
            {
                break;
            }
        }
    }
}

void tivxRemoveTargetKernelErode3x3()
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i < sizeof(gTivxFilt3x3KernelInfo)/
            sizeof(tivxFilter3x3KernelInfo); i ++)
    {
        status = tivxRemoveTargetKernel(gTivxFilt3x3KernelInfo[i].target_kernel);

        if (VX_SUCCESS == status)
        {
            gTivxFilt3x3KernelInfo[i].target_kernel = NULL;
        }
    }
}

