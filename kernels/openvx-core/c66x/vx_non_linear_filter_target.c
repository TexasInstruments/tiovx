/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/



#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_non_linear_filter.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

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
    uint8_t *src_addr;
    uint8_t *dst_addr;
    uint8_t *mask_addr;

    if ((num_params != TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX]))
    {
        status = VX_FAILURE;
    }
    else
    {
        void *src_desc_target_ptr;
        void *mask_desc_target_ptr;
        void *dst_desc_target_ptr;

        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];
        src_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX];
        mask_desc = (tivx_obj_desc_matrix_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX];

        src_desc_target_ptr = tivxMemShared2TargetPtr(
          src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_heap_region);
        mask_desc_target_ptr = tivxMemShared2TargetPtr(
          mask_desc->mem_ptr.shared_ptr, mask_desc->mem_ptr.mem_heap_region);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(
          dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(src_desc_target_ptr,
           src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(mask_desc_target_ptr,
           mask_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
           dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        mask_addr = (uint8_t *)((uintptr_t)mask_desc_target_ptr);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

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

        tivxMemBufferUnmap(src_desc_target_ptr,
           src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(mask_desc_target_ptr,
           mask_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc_target_ptr,
           dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
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

            temp_ptr = tivxMemAlloc(mask_desc->columns*mask_desc->rows*2*
                sizeof(int64_t), TIVX_MEM_EXTERNAL);

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
                tivxMemFree(temp_ptr, temp_ptr_size, TIVX_MEM_EXTERNAL);
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

void tivxAddTargetKernelNonLinearFilter(void)
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

void tivxRemoveTargetKernelNonLinearFilter(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_non_linear_filter_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_non_linear_filter_target_kernel = NULL;
    }
}


