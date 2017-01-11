/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_halfscale_gaussian.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <stdio.h>

static tivx_target_kernel vx_halfscale_gaussian_target_kernel = NULL;

vx_status VX_CALLBACK tivxHalfscaleGaussian(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_image_t *dst_desc;
    tivx_obj_desc_scalar_t *gsize_desc;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t *src_addr, *dst_addr;
    vx_rectangle_t rect;

    if ( num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        int32_t gsize_value = 1;

        src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_DST_IDX];
        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(src_desc->mem_ptr[0].target_ptr,
           src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
           dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));

        dst_addr = (uint8_t *)((uintptr_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst_desc->imagepatch_addr[0U]));

        vxlib_src.dim_x = src_desc->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y;
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if(gsize_value == 1) {

            status |= VXLIB_scaleImageNearest_i8u_o8u(src_addr, &vxlib_src,
                                                      dst_addr, &vxlib_dst,
                                                      2, 2, 0, 0, 0, 0);

        } else if (gsize_value == 3 || gsize_value == 5) {

            VXLIB_bufParams2D_t gauss_params;
            uint8_t *pGauss;

            gauss_params.dim_x    = vxlib_src.dim_x;
            gauss_params.dim_y    = vxlib_src.dim_y-(gsize_value-1);
            gauss_params.stride_y = vxlib_src.stride_y;

            if (gsize_value == 3) {
                void *gaussOut;
                uint32_t gaussOut_size;
                status = tivxGetTargetKernelInstanceContext(kernel, &gaussOut, &gaussOut_size);
                if (VX_SUCCESS == status)
                {
                    pGauss = (uint8_t*)((uint8_t*)gaussOut + gauss_params.stride_y + 1);
                    status |= VXLIB_gaussian_3x3_i8u_o8u(src_addr, &vxlib_src,
                                                     pGauss, &gauss_params);
                    status |= VXLIB_scaleImageNearest_i8u_o8u((uint8_t*)gaussOut, &gauss_params,
                                                          dst_addr, &vxlib_dst,
                                                          2, 2, 0, 0, 0, 0);
                }
            } else {
                pGauss = (uint8_t*)(dst_addr + vxlib_dst.stride_y + 1);
                vxlib_dst.dim_y -= 2;
                status |= VXLIB_halfScaleGaussian_5x5_i8u_o8u(src_addr, &vxlib_src,
                                                              pGauss, &vxlib_dst);
            }

        }

        tivxMemBufferUnmap(src_desc->mem_ptr[0].target_ptr,
           src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
           dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);
    }

    return status;
}

vx_status VX_CALLBACK tivxHalfscaleGaussianCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_scalar_t *gsize_desc;
    int32_t gsize_value = 0;

    if (num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS; i ++)
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

        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if (gsize_value == 3)
        {

            src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_SRC_IDX];

            temp_ptr = tivxMemAlloc(src_desc->imagepatch_addr[0].stride_y *
                src_desc->imagepatch_addr[0].dim_y);

            if (NULL == temp_ptr)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                memset(temp_ptr, 0, src_desc->imagepatch_addr[0].stride_y *
                    src_desc->imagepatch_addr[0].dim_y);
                tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                    (src_desc->imagepatch_addr[0].stride_y *
                    src_desc->imagepatch_addr[0].dim_y) );
            }
        }
    }

    return (status);
}

vx_status VX_CALLBACK tivxHalfscaleGaussianDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;
    tivx_obj_desc_scalar_t *gsize_desc;
    int32_t gsize_value = 0;

    if (num_params != TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HALFSCALE_GAUSSIAN_MAX_PARAMS; i ++)
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

        gsize_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_HALFSCALE_GAUSSIAN_GSIZE_IDX];

        if( gsize_desc != NULL)
        {
            gsize_value = gsize_desc->data.s32;
        }

        if (gsize_value == 3)
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
    }

    return (status);
}

vx_status VX_CALLBACK tivxHalfscaleGaussianControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelHalfscaleGaussian()
{
    vx_status status = VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ( self_cpu == TIVX_CPU_ID_DSP1 )
    {
        strncpy(target_name, TIVX_TARGET_DSP1, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    if ( self_cpu == TIVX_CPU_ID_DSP2 )
    {
        strncpy(target_name, TIVX_TARGET_DSP2, TIVX_TARGET_MAX_NAME);
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        vx_halfscale_gaussian_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_HALFSCALE_GAUSSIAN,
                            target_name,
                            tivxHalfscaleGaussian,
                            tivxHalfscaleGaussianCreate,
                            tivxHalfscaleGaussianDelete,
                            tivxHalfscaleGaussianControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelHalfscaleGaussian()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_halfscale_gaussian_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_halfscale_gaussian_target_kernel = NULL;
    }
}


