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
#include <tivx_kernel_convolve.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_convolve_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelConvolveProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_convolution_t *conv;
    vx_uint8 *src_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_CONVOLVE_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_CONVOLVE_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CONVOLVE_IN_IMG_IDX];
        conv = (tivx_obj_desc_convolution_t *)obj_desc[
            TIVX_KERNEL_CONVOLVE_IN_CONVOLVE_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CONVOLVE_OUT_IMG_IDX];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);
        conv->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            conv->mem_ptr.shared_ptr, conv->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(conv->mem_ptr.target_ptr, conv->mem_size,
            conv->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x + (conv->columns/2U),
            rect.start_y + (conv->rows/2U),
            &dst->imagepatch_addr[0]));


        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        /* All filter reduces the output size, therefore reduce output
         * height, but leave output width the same (DSP optimization) */
        vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y -
            (conv->rows/2U) * 2U;
        vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
        if (VX_DF_IMAGE_U8 == dst->format)
        {
            vxlib_dst.data_type = VXLIB_UINT8;
        }
        else
        {
            vxlib_dst.data_type = VXLIB_INT16;
        }

        if (vxlib_dst.data_type == VXLIB_UINT8)
        {
            status = VXLIB_convolve_i8u_c16s_o8u(src_addr, &vxlib_src,
                dst_addr, &vxlib_dst, conv->mem_ptr.target_ptr,
                conv->columns, conv->rows, conv->scale);
        }
        else
        {
            status = VXLIB_convolve_i8u_c16s_o16s(src_addr, &vxlib_src,
                (int16_t*)dst_addr, &vxlib_dst, conv->mem_ptr.target_ptr,
                conv->columns, conv->rows, conv->scale);
        }
        if (VXLIB_SUCCESS != status)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(conv->mem_ptr.target_ptr, conv->mem_size,
            conv->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelConvolveCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvolveDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelConvolveControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelConvolve()
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

        vx_convolve_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CUSTOM_CONVOLUTION,
            target_name,
            tivxKernelConvolveProcess,
            tivxKernelConvolveCreate,
            tivxKernelConvolveDelete,
            tivxKernelConvolveControl,
            NULL);
    }
}


void tivxRemoveTargetKernelConvolve()
{
    if (NULL != vx_convolve_target_kernel)
    {
        tivxRemoveTargetKernel(vx_convolve_target_kernel);
        vx_convolve_target_kernel = NULL;
    }
}
