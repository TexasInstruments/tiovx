/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_non_linear_filter.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>

static tivx_target_kernel vx_non_linear_filter_target_kernel = NULL;

vx_status VX_CALLBACK tivxNonLinearFilter(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_scalar_t *function_desc;
    tivx_obj_desc_image_t *src_desc;
    tivx_obj_desc_matrix_t *mask_desc;
    tivx_obj_desc_image_t *dst_desc;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst, mask_params;
    vx_rectangle_t rect;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    uint8_t *mask_addr;

    if ( num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX])
    )
    {
        status = VX_FAILURE;
    }
    else
    {
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];
        src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX];
        mask_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX];

        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        mask_desc->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
          mask_desc->mem_ptr.shared_ptr, mask_desc->mem_ptr.mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
          dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        tivxMemBufferMap(src_desc->mem_ptr[0].target_ptr,
           src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(mask_desc->mem_ptr.target_ptr,
           mask_desc->mem_size, mask_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
           dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        mask_addr = (uint8_t *)((uintptr_t)mask_desc->mem_ptr.target_ptr);

        rect = src_desc->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src_desc->imagepatch_addr[0U]));

        dst_addr = (uint8_t *)((uintptr_t)dst_desc->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(
                    rect.start_x + (mask_desc->origin_x),
                    rect.start_y + (mask_desc->origin_y),
            &dst_desc->imagepatch_addr[0U]));


        vxlib_src.dim_x = src_desc->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src_desc->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src_desc->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = VXLIB_UINT8;

        vxlib_dst.dim_x = dst_desc->imagepatch_addr[0].dim_x;
        vxlib_dst.dim_y = dst_desc->imagepatch_addr[0].dim_y - (2*(mask_desc->origin_y));
        vxlib_dst.stride_y = dst_desc->imagepatch_addr[0].stride_y;
        vxlib_dst.data_type = VXLIB_UINT8;

        mask_params.dim_x    = mask_desc->columns;
        mask_params.dim_y    = mask_desc->rows;
        mask_params.stride_y = mask_desc->columns;

        if (VX_NONLINEAR_FILTER_MIN == function_desc->data.enm)
        {
            status |= VXLIB_erode_MxN_i8u_i8u_o8u(src_addr, &vxlib_src,
                                                  dst_addr, &vxlib_dst,
                                                  mask_addr, &mask_params);
        }
        else if (VX_NONLINEAR_FILTER_MAX == function_desc->data.enm)
        {
            status |= VXLIB_dilate_MxN_i8u_i8u_o8u(src_addr, &vxlib_src,
                                                   dst_addr, &vxlib_dst,
                                                   mask_addr, &mask_params);
        }
        else
        {
            void *scratch;
            uint32_t scratch_size;
            status = tivxGetTargetKernelInstanceContext(
                            kernel,
                            &scratch, &scratch_size);

            if(status==VX_SUCCESS)
            {
                status |= VXLIB_median_MxN_i8u_i8u_o8u(
                                        src_addr, &vxlib_src,
                                        dst_addr, &vxlib_dst,
                                        mask_addr, &mask_params,
                                        (int64_t*)scratch, scratch_size);
            }
        }

        tivxMemBufferUnmap(src_desc->mem_ptr[0].target_ptr,
           src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(mask_desc->mem_ptr.target_ptr,
           mask_desc->mem_size, mask_desc->mem_ptr.mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
           dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);


    }

    return status;
}

vx_status VX_CALLBACK tivxNonLinearFilterCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_scalar_t *function_desc;
    tivx_obj_desc_matrix_t *mask_desc;

    if (num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS; i ++)
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
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];

        if ( (VX_NONLINEAR_FILTER_MIN != function_desc->data.enm) &&
             (VX_NONLINEAR_FILTER_MAX != function_desc->data.enm) )
        {
            mask_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];

            temp_ptr = tivxMemAlloc(mask_desc->columns*mask_desc->rows*2*sizeof(int64_t));

            if (NULL == temp_ptr)
            {
                status = VX_ERROR_NO_MEMORY;
            }
            else
            {
                memset(temp_ptr, 0, mask_desc->columns*mask_desc->rows*2*sizeof(int64_t));
                tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                    mask_desc->columns*mask_desc->rows*2*sizeof(int64_t));
            }
        }
    }

    return (status);
}

vx_status VX_CALLBACK tivxNonLinearFilterDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    tivx_obj_desc_scalar_t *function_desc;
    uint32_t temp_ptr_size;

    if (num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS; i ++)
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
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];

        if ( (VX_NONLINEAR_FILTER_MIN != function_desc->data.enm) &&
             (VX_NONLINEAR_FILTER_MAX != function_desc->data.enm) )
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

vx_status VX_CALLBACK tivxNonLinearFilterControl(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

void tivxAddTargetKernelNonLinearFilter()
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
        vx_non_linear_filter_target_kernel = tivxAddTargetKernel(
                            VX_KERNEL_NON_LINEAR_FILTER,
                            target_name,
                            tivxNonLinearFilter,
                            tivxNonLinearFilterCreate,
                            tivxNonLinearFilterDelete,
                            tivxNonLinearFilterControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelNonLinearFilter()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_non_linear_filter_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_non_linear_filter_target_kernel = NULL;
    }
}


